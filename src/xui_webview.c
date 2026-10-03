#include "../xui_config.h"
#if XUI_ENABLE_WEBVIEW
/* Generic XUI WebView widget. WebView2 is an optional Windows backend. */
#include "xui_webview_internal.h"
#include <stdlib.h>
#include <string.h>

#define XUI_WEBVIEW_MAX_MESSAGE_BYTES (1024u * 1024u)

#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>
#include "xge.h"
#include "WebView2.h"
#endif

typedef struct xui_webview_data xui_webview_data;
typedef struct xui_web_request_t xui_web_request_t;
enum { WEB_REQUEST_SCRIPT = 1, WEB_REQUEST_PNG = 2 };
struct xui_web_request_t {
    int refs;
    int kind;
    int state;
    int32_t error;
    void* result;
    size_t bytes;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    DWORD thread;
    uint64_t epoch;
    xui_webview_data* owner;
    xui_web_request_t* next;
#endif
};
struct xui_webview_data {
    xui_widget widget;
    xui_webview_event_proc on_event;
    void* user;
    xui_webview_message_proc on_message;
    void* message_user;
    char* folder;
    int state;
    int32_t error;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    LONG refs;
    DWORD thread;
    int com_initialized;
    HWND parent;
    HWND host;
    RECT host_rect;
    int host_visible;
    RECT controller_bounds;
    int controller_bounds_valid;
    int controller_visible;
    ICoreWebView2Controller* controller;
    ICoreWebView2* view;
    uint64_t page_epoch;
    int page_loaded;
    xui_web_request_t* requests;
    LONG inflight_scripts;
    LONG inflight_captures;
    wchar_t* message_origin;
    EventRegistrationToken message_token;
    int message_registered;
    EventRegistrationToken resource_token;
    int resource_registered;
    ICoreWebView2Environment* resource_environment;
    int block_external_resources;
    uint32_t blocked_resource_count;
    EventRegistrationToken navigation_starting_token;
    int navigation_starting_registered;
    EventRegistrationToken navigation_token;
    int navigation_registered;
    EventRegistrationToken process_token;
    int process_registered;
    int process_failed_pending;
    EventRegistrationToken focus_gained_token;
    EventRegistrationToken focus_lost_token;
    EventRegistrationToken move_focus_token;
    int focus_gained_registered;
    int focus_lost_registered;
    int move_focus_registered;
#endif
};

static void web_request_retain(xui_web_request_t* request)
{
    if (request) request->refs++;
}

static void web_request_release(xui_web_request_t* request)
{
    if (request && --request->refs == 0) {
        free(request->result);
        free(request);
    }
}

static char* webview_copy_string(const char* source)
{
    size_t bytes;
    char* copy;
    if (!source) return NULL;
    bytes = strlen(source);
    if (bytes == SIZE_MAX) return NULL;
    copy = malloc(bytes + 1);
    if (copy) memcpy(copy, source, bytes + 1);
    return copy;
}

static xui_webview_data* webview_get(xui_widget widget)
{
    xui_widget_type type;
    xui_webview_data** slot;
    if (!widget) return NULL;
    type = xuiWidgetFindType(xuiWidgetGetContext(widget), "webview");
    if (!type || !xuiWidgetIsType(widget, type)) return NULL;
    slot = xuiWidgetGetTypeData(widget);
    return slot ? *slot : NULL;
}

static void webview_emit(xui_webview_data* data, int event, int32_t error)
{
    xui_webview_event_proc callback = data->on_event;
    xui_widget widget = data->widget;
    void* user = data->user;
    if (callback && widget) callback(widget, event, error, user);
}

#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
static void webview_retain(xui_webview_data* data)
{
    InterlockedIncrement(&data->refs);
}

static void webview_release(xui_webview_data* data)
{
    if (InterlockedDecrement(&data->refs) == 0) {
        if (data->com_initialized) CoUninitialize();
        free(data->message_origin);
        free(data->folder);
        free(data);
    }
}

static wchar_t* webview_utf16(const char* utf8, size_t bytes)
{
    int count;
    wchar_t* result;
    if (!utf8 || bytes > INT_MAX || memchr(utf8, 0, bytes)) return NULL;
    count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, (int)bytes, NULL, 0);
    if (!count && bytes) return NULL;
    result = calloc((size_t)count + 1, sizeof(*result));
    if (!result) return NULL;
    if (count && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8,
            (int)bytes, result, count) != count) {
        free(result);
        return NULL;
    }
    return result;
}

#define XUI_WEBVIEW_MAX_SCRIPT_BYTES (16u * 1024u * 1024u)
#define XUI_WEBVIEW_MAX_CAPTURE_BYTES (64u * 1024u * 1024u)
#define XUI_WEBVIEW_MAX_CAPTURE_EDGE 4096

static HRESULT webview_utf8_result(const wchar_t* wide, char** out, size_t* bytes)
{
    size_t length = 0;
    int count;
    char* result;
    *out = NULL;
    *bytes = 0;
    if (!wide) return E_INVALIDARG;
    while (length <= XUI_WEBVIEW_MAX_SCRIPT_BYTES && wide[length]) length++;
    if (length > XUI_WEBVIEW_MAX_SCRIPT_BYTES)
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    count = length ? WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide,
        (int)length, NULL, 0, NULL, NULL) : 0;
    if (length && !count) return HRESULT_FROM_WIN32(GetLastError());
    if ((size_t)count > XUI_WEBVIEW_MAX_SCRIPT_BYTES)
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    result = malloc((size_t)count + 1);
    if (!result) return E_OUTOFMEMORY;
    if (count && WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide,
            (int)length, result, count, NULL, NULL) != count) {
        free(result);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    result[count] = 0;
    *out = result;
    *bytes = (size_t)count;
    return S_OK;
}

static int webview_valid_virtual_host(const char* host)
{
    size_t length = 0, label = 0;
    unsigned char previous = 0;
    if (!host || !*host) return 0;
    while (host[length]) {
        unsigned char c = (unsigned char)host[length];
        if (c == '.') {
            if (!label || previous == '-') return 0;
            label = 0;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') || c == '-') {
            if (!label && c == '-') return 0;
            if (++label > 63) return 0;
        } else {
            return 0;
        }
        previous = c;
        if (++length > 253) return 0;
    }
    return label && previous != '-';
}

static int webview_valid_message_origin(const char* origin)
{
    if (!origin) return 0;
    /* NavigateToString uses this exact top-level source. Only private
     * Document hosts opt into a handler for it. */
    if (strcmp(origin, "about:blank") == 0) return 1;
    if (strncmp(origin, "https://", 8) == 0) return webview_valid_virtual_host(origin + 8);
    if (strncmp(origin, "http://", 7) == 0) return webview_valid_virtual_host(origin + 7);
    return 0;
}

static int webview_origin_matches(const wchar_t* source, const wchar_t* origin)
{
    size_t length;
    wchar_t next;
    if (!source || !origin) return 0;
    length = wcslen(origin);
    if (_wcsnicmp(source, origin, length) != 0) return 0;
    next = source[length];
    return !next || next == L'/' || next == L'?' || next == L'#';
}

static wchar_t* webview_absolute_folder(const char* utf8)
{
    wchar_t* wide;
    wchar_t full[MAX_PATH];
    wchar_t* result;
    DWORD length, attributes;
    size_t bytes;
    if (!utf8) return NULL;
    wide = webview_utf16(utf8, strlen(utf8));
    if (!wide) return NULL;
    if (!(((wide[0] >= L'A' && wide[0] <= L'Z') ||
            (wide[0] >= L'a' && wide[0] <= L'z')) && wide[1] == L':' &&
            (wide[2] == L'\\' || wide[2] == L'/')) &&
            !(wide[0] == L'\\' && wide[1] == L'\\')) {
        free(wide);
        return NULL;
    }
    length = GetFullPathNameW(wide, MAX_PATH, full, NULL);
    free(wide);
    if (!length || length >= MAX_PATH) return NULL;
    attributes = GetFileAttributesW(full);
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return NULL;
    bytes = ((size_t)length + 1) * sizeof(*result);
    result = malloc(bytes);
    if (result) memcpy(result, full, bytes);
    return result;
}

static int webview_on_owner_thread(const xui_webview_data* data)
{
    return !data->thread || data->thread == GetCurrentThreadId();
}

static void web_request_finish(xui_web_request_t* request, int state,
    HRESULT error, void* result, size_t bytes)
{
    xui_webview_data* data = request->owner;
    xui_web_request_t** link;
    if (request->state != XUI_WEB_REQUEST_PENDING || !data) {
        free(result);
        return;
    }
    for (link = &data->requests; *link && *link != request; link = &(*link)->next) {}
    if (*link == request) {
        *link = request->next;
    }
    request->next = NULL;
    request->owner = NULL;
    request->state = state;
    request->error = (int32_t)error;
    request->result = result;
    request->bytes = bytes;
}

static void webview_cancel_requests(xui_webview_data* data)
{
    while (data->requests)
        web_request_finish(data->requests, XUI_WEB_REQUEST_CANCELLED, E_ABORT, NULL, 0);
}

static void webview_cancel_captures(xui_webview_data* data)
{
    xui_web_request_t* request = data->requests;
    while (request) {
        xui_web_request_t* next = request->next;
        if (request->kind == WEB_REQUEST_PNG)
            web_request_finish(request, XUI_WEB_REQUEST_CANCELLED, E_ABORT, NULL, 0);
        request = next;
    }
}

static void webview_advance_page(xui_webview_data* data)
{
    webview_cancel_requests(data);
    data->page_loaded = 0;
    if (++data->page_epoch == 0) data->page_epoch = 1;
}

static void webview_stop_backend(xui_webview_data* data)
{
    HWND focused = GetFocus();
    webview_advance_page(data);
    if (data->host && focused && (focused == data->host || IsChild(data->host, focused)) &&
            data->parent && IsWindow(data->parent)) SetFocus(data->parent);
    if (data->controller && data->focus_gained_registered) {
        ICoreWebView2Controller_remove_GotFocus(data->controller, data->focus_gained_token);
        data->focus_gained_registered = 0;
    }
    if (data->controller && data->focus_lost_registered) {
        ICoreWebView2Controller_remove_LostFocus(data->controller, data->focus_lost_token);
        data->focus_lost_registered = 0;
    }
    if (data->controller && data->move_focus_registered) {
        ICoreWebView2Controller_remove_MoveFocusRequested(data->controller, data->move_focus_token);
        data->move_focus_registered = 0;
    }
    if (data->view && data->navigation_registered) {
        ICoreWebView2_remove_NavigationCompleted(data->view, data->navigation_token);
        data->navigation_registered = 0;
    }
    if (data->view && data->navigation_starting_registered) {
        ICoreWebView2_remove_NavigationStarting(data->view, data->navigation_starting_token);
        data->navigation_starting_registered = 0;
    }
    if (data->view && data->process_registered) {
        ICoreWebView2_remove_ProcessFailed(data->view, data->process_token);
        data->process_registered = 0;
    }
    if (data->view && data->message_registered) {
        ICoreWebView2_remove_WebMessageReceived(data->view, data->message_token);
        data->message_registered = 0;
    }
    if (data->view && data->resource_registered) {
        ICoreWebView2_remove_WebResourceRequested(data->view, data->resource_token);
        data->resource_registered = 0;
    }
    if (data->resource_environment) {
        ICoreWebView2Environment_Release(data->resource_environment);
        data->resource_environment = NULL;
    }
    if (data->controller) ICoreWebView2Controller_Close(data->controller);
    if (data->view) { ICoreWebView2_Release(data->view); data->view = NULL; }
    if (data->controller) {
        ICoreWebView2Controller_Release(data->controller);
        data->controller = NULL;
    }
    if (data->host) { DestroyWindow(data->host); data->host = NULL; }
    data->process_failed_pending = 0;
}

static void webview_failed(xui_webview_data* data, HRESULT error)
{
    if (data->state != XUI_WEBVIEW_INITIALIZING) return;
    data->state = XUI_WEBVIEW_FAILED;
    data->error = (int32_t)error;
    webview_stop_backend(data);
    webview_emit(data, XUI_WEBVIEW_EVENT_FAILED, data->error);
}

static int webview_intersect(xui_rect_t* rect, xui_rect_t clip)
{
    int right = rect->fX + rect->fW;
    int bottom = rect->fY + rect->fH;
    int clip_right = clip.fX + clip.fW;
    int clip_bottom = clip.fY + clip.fH;
    if (rect->fX < clip.fX) rect->fX = clip.fX;
    if (rect->fY < clip.fY) rect->fY = clip.fY;
    if (right > clip_right) right = clip_right;
    if (bottom > clip_bottom) bottom = clip_bottom;
    rect->fW = right - rect->fX;
    rect->fH = bottom - rect->fY;
    return rect->fW > 0 && rect->fH > 0;
}

static void webview_sync_host(xui_webview_data* data)
{
    xui_rect_t world, visible, clip;
    xui_widget parent;
    RECT client, host_rect, bounds;
    int is_visible, is_enabled;
    if (!data->host || !data->widget || !IsWindow(data->parent)) return;
    is_enabled = xuiWidgetGetEffectiveEnabled(data->widget);
    if (!!IsWindowEnabled(data->host) != !!is_enabled) {
        HWND focused = GetFocus();
        if (!is_enabled && focused &&
                (focused == data->host || IsChild(data->host, focused)))
            SetFocus(data->parent);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
        EnableWindow(data->host, is_enabled);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
        if (!is_enabled &&
                xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) == data->widget)
            (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), NULL);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
    }
    world = xuiWidgetGetWorldRect(data->widget);
    visible = world;
    GetClientRect(data->parent, &client);
    clip = (xui_rect_t){0, 0, client.right, client.bottom};
    is_visible = xuiWidgetIsAttachedToContext(data->widget) &&
        xuiWidgetGetEffectiveVisible(data->widget) &&
        webview_intersect(&visible, clip);
    for (parent = xuiWidgetGetParent(data->widget); is_visible && parent;
            parent = xuiWidgetGetParent(parent)) {
        int overflow = xuiWidgetGetOverflow(parent);
        if (overflow == XUI_OVERFLOW_HIDDEN || overflow == XUI_OVERFLOW_CLIP)
            is_visible = webview_intersect(&visible, xuiWidgetGetWorldRect(parent));
    }
    if (!is_visible || world.fW <= 0 || world.fH <= 0) {
        webview_cancel_captures(data);
        HWND focused = GetFocus();
        if (focused && (focused == data->host || IsChild(data->host, focused)))
            SetFocus(data->parent);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
        if (data->widget &&
                xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) == data->widget)
            (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), NULL);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
        if (data->host_visible) ShowWindow(data->host, SW_HIDE);
        data->host_visible = 0;
        if (data->controller && data->controller_visible) {
            ICoreWebView2Controller_put_IsVisible(data->controller, FALSE);
            if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
            data->controller_visible = 0;
        }
        return;
    }
    host_rect.left = visible.fX;
    host_rect.top = visible.fY;
    host_rect.right = visible.fX + visible.fW;
    host_rect.bottom = visible.fY + visible.fH;
    if (memcmp(&host_rect, &data->host_rect, sizeof(host_rect))) {
        SetWindowPos(data->host, NULL, host_rect.left, host_rect.top,
            host_rect.right - host_rect.left, host_rect.bottom - host_rect.top,
            SWP_NOACTIVATE | SWP_NOZORDER);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
        data->host_rect = host_rect;
        if (data->controller)
            ICoreWebView2Controller_NotifyParentWindowPositionChanged(data->controller);
        if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
    }
    bounds.left = world.fX - host_rect.left;
    bounds.top = world.fY - host_rect.top;
    bounds.right = bounds.left + world.fW;
    bounds.bottom = bounds.top + world.fH;
    if (data->controller) {
        if (!data->controller_bounds_valid ||
                memcmp(&bounds, &data->controller_bounds, sizeof(bounds))) {
            ICoreWebView2Controller_put_Bounds(data->controller, bounds);
            if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
            data->controller_bounds = bounds;
            data->controller_bounds_valid = 1;
        }
        if (!data->controller_visible) {
            ICoreWebView2Controller_put_IsVisible(data->controller, TRUE);
            if (!data->host || data->state == XUI_WEBVIEW_CLOSED) return;
            data->controller_visible = 1;
        }
    }
    if (!data->host_visible) ShowWindow(data->host, SW_SHOWNA);
    data->host_visible = 1;
    /* The HWND can gain focus without a matching controller GotFocus callback. */
    if (data->state == XUI_WEBVIEW_READY && is_enabled &&
            xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) != data->widget) {
        HWND focused = GetFocus();
        if (focused && (focused == data->host || IsChild(data->host, focused)))
            (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), data->widget);
    }
}

typedef struct webview_environment_handler {
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_environment_handler;
typedef struct webview_controller_handler {
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_controller_handler;
typedef struct webview_navigation_handler {
    ICoreWebView2NavigationCompletedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_navigation_handler;
typedef struct webview_navigation_starting_handler {
    ICoreWebView2NavigationStartingEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_navigation_starting_handler;
typedef struct webview_process_handler {
    ICoreWebView2ProcessFailedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_process_handler;
typedef struct webview_script_handler {
    ICoreWebView2ExecuteScriptCompletedHandler iface;
    LONG refs;
    LONG inflight;
    xui_webview_data* data;
    xui_web_request_t* request;
} webview_script_handler;
typedef struct webview_message_handler {
    ICoreWebView2WebMessageReceivedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_message_handler;
typedef struct webview_resource_handler {
    ICoreWebView2WebResourceRequestedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_resource_handler;
typedef struct webview_capture_handler {
    ICoreWebView2CapturePreviewCompletedHandler iface;
    LONG refs;
    LONG inflight;
    xui_webview_data* data;
    xui_web_request_t* request;
    IStream* stream;
} webview_capture_handler;
typedef struct webview_focus_handler {
    ICoreWebView2FocusChangedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
    int gained;
} webview_focus_handler;
typedef struct webview_move_focus_handler {
    ICoreWebView2MoveFocusRequestedEventHandler iface;
    LONG refs;
    xui_webview_data* data;
} webview_move_focus_handler;

#define WEBVIEW_HANDLER_METHODS(Name, Type, Interface, Iid) \
    static HRESULT STDMETHODCALLTYPE Name##_query(Interface* self, REFIID iid, void** out) \
    { \
        if (!out) return E_POINTER; \
        *out = NULL; \
        if (!IsEqualIID(iid, &IID_IUnknown) && !IsEqualIID(iid, &Iid)) return E_NOINTERFACE; \
        *out = self; \
        Name##_add(self); \
        return S_OK; \
    } \
    static ULONG STDMETHODCALLTYPE Name##_add(Interface* self) \
    { return (ULONG)InterlockedIncrement(&((Type*)self)->refs); } \
    static ULONG STDMETHODCALLTYPE Name##_release(Interface* self) \
    { \
        Type* handler = (Type*)self; \
        LONG refs = InterlockedDecrement(&handler->refs); \
        if (!refs) { webview_release(handler->data); free(handler); } \
        return (ULONG)refs; \
    }

/* AddRef precedes QueryInterface because the latter delegates to it. */
#define WEBVIEW_HANDLER_DECL(Name, Type, Interface, Iid) \
    static ULONG STDMETHODCALLTYPE Name##_add(Interface* self); \
    WEBVIEW_HANDLER_METHODS(Name, Type, Interface, Iid)

WEBVIEW_HANDLER_DECL(environment, webview_environment_handler,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
    IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)
WEBVIEW_HANDLER_DECL(controller, webview_controller_handler,
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
    IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)
WEBVIEW_HANDLER_DECL(navigation, webview_navigation_handler,
    ICoreWebView2NavigationCompletedEventHandler,
    IID_ICoreWebView2NavigationCompletedEventHandler)
WEBVIEW_HANDLER_DECL(navigation_starting, webview_navigation_starting_handler,
    ICoreWebView2NavigationStartingEventHandler,
    IID_ICoreWebView2NavigationStartingEventHandler)
WEBVIEW_HANDLER_DECL(process, webview_process_handler,
    ICoreWebView2ProcessFailedEventHandler,
    IID_ICoreWebView2ProcessFailedEventHandler)
WEBVIEW_HANDLER_DECL(message, webview_message_handler,
    ICoreWebView2WebMessageReceivedEventHandler,
    IID_ICoreWebView2WebMessageReceivedEventHandler)
WEBVIEW_HANDLER_DECL(resource, webview_resource_handler,
    ICoreWebView2WebResourceRequestedEventHandler,
    IID_ICoreWebView2WebResourceRequestedEventHandler)
WEBVIEW_HANDLER_DECL(focus, webview_focus_handler,
    ICoreWebView2FocusChangedEventHandler,
    IID_ICoreWebView2FocusChangedEventHandler)
WEBVIEW_HANDLER_DECL(move_focus, webview_move_focus_handler,
    ICoreWebView2MoveFocusRequestedEventHandler,
    IID_ICoreWebView2MoveFocusRequestedEventHandler)

static HRESULT STDMETHODCALLTYPE focus_invoke(ICoreWebView2FocusChangedEventHandler* self,
    ICoreWebView2Controller* sender, IUnknown* args)
{
    webview_focus_handler* handler = (webview_focus_handler*)self;
    xui_webview_data* data = handler->data;
    xui_widget widget = data->widget;
    xui_context context;
    HWND focused;
    int in_browser;
    (void)sender; (void)args;
    if (data->state != XUI_WEBVIEW_READY || !widget) return S_OK;
    context = xuiWidgetGetContext(widget);
    focused = GetFocus();
    in_browser = data->host && focused &&
        (focused == data->host || IsChild(data->host, focused));
    if (handler->gained) {
        if (in_browser && data->host_visible &&
                xuiWidgetIsAttachedToContext(widget) &&
                xuiWidgetGetEffectiveVisible(widget) &&
                xuiWidgetGetEffectiveEnabled(widget) &&
                xuiGetFocusWidget(context) != widget)
            (void)xuiSetFocusWidget(context, widget);
    } else if (!in_browser && xuiGetFocusWidget(context) == widget) {
        (void)xuiSetFocusWidget(context, NULL);
    }
    return S_OK;
}

static ICoreWebView2FocusChangedEventHandlerVtbl focus_vtable = {
    focus_query, focus_add, focus_release, focus_invoke
};

static HRESULT STDMETHODCALLTYPE move_focus_invoke(
    ICoreWebView2MoveFocusRequestedEventHandler* self,
    ICoreWebView2Controller* sender, ICoreWebView2MoveFocusRequestedEventArgs* args)
{
    xui_webview_data* data = ((webview_move_focus_handler*)self)->data;
    xui_widget widget = data->widget;
    xui_context context;
    COREWEBVIEW2_MOVE_FOCUS_REASON reason;
    (void)sender;
    if (data->state != XUI_WEBVIEW_READY || !widget ||
            FAILED(ICoreWebView2MoveFocusRequestedEventArgs_get_Reason(args, &reason)) ||
            (reason != COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT &&
             reason != COREWEBVIEW2_MOVE_FOCUS_REASON_PREVIOUS)) return S_OK;
    context = xuiWidgetGetContext(widget);
    if (xuiGetFocusWidget(context) != widget &&
            xuiSetFocusWidget(context, widget) != XUI_OK) return S_OK;
    if (data->widget != widget || data->state != XUI_WEBVIEW_READY) return S_OK;
    if (xuiFocusNext(context, reason == COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT) == XUI_OK &&
            data->widget && xuiGetFocusWidget(context) != widget) {
        ICoreWebView2MoveFocusRequestedEventArgs_put_Handled(args, TRUE);
        if (data->parent && IsWindow(data->parent)) SetFocus(data->parent);
    }
    return S_OK;
}

static ICoreWebView2MoveFocusRequestedEventHandlerVtbl move_focus_vtable = {
    move_focus_query, move_focus_add, move_focus_release, move_focus_invoke
};

static HRESULT STDMETHODCALLTYPE navigation_starting_invoke(
    ICoreWebView2NavigationStartingEventHandler* self, ICoreWebView2* sender,
    ICoreWebView2NavigationStartingEventArgs* args)
{
    xui_webview_data* data = ((webview_navigation_starting_handler*)self)->data;
    (void)sender; (void)args;
    if (data->state == XUI_WEBVIEW_READY) webview_advance_page(data);
    return S_OK;
}

static ICoreWebView2NavigationStartingEventHandlerVtbl navigation_starting_vtable = {
    navigation_starting_query, navigation_starting_add, navigation_starting_release,
    navigation_starting_invoke
};

static HRESULT STDMETHODCALLTYPE process_invoke(
    ICoreWebView2ProcessFailedEventHandler* self, ICoreWebView2* sender,
    ICoreWebView2ProcessFailedEventArgs* args)
{
    xui_webview_data* data = ((webview_process_handler*)self)->data;
    COREWEBVIEW2_PROCESS_FAILED_KIND kind;
    HRESULT hr;
    (void)sender;
    if (data->state != XUI_WEBVIEW_READY) return S_OK;
    hr = ICoreWebView2ProcessFailedEventArgs_get_ProcessFailedKind(args, &kind);
    if (FAILED(hr) || (kind != COREWEBVIEW2_PROCESS_FAILED_KIND_BROWSER_PROCESS_EXITED &&
        kind != COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED))
        return S_OK;
    /* Only the browser and main-frame renderer are fatal to this widget.
     * WebView2 recovers GPU/utility failures itself. Defer COM teardown and
     * the host callback until XUI's next update, outside this COM callback. */
    data->state = XUI_WEBVIEW_FAILED;
    data->error = E_FAIL;
    data->process_failed_pending = 1;
    webview_advance_page(data);
    return S_OK;
}

static ICoreWebView2ProcessFailedEventHandlerVtbl process_vtable = {
    process_query, process_add, process_release, process_invoke
};

static HRESULT STDMETHODCALLTYPE message_invoke(
    ICoreWebView2WebMessageReceivedEventHandler* self, ICoreWebView2* sender,
    ICoreWebView2WebMessageReceivedEventArgs* args)
{
    xui_webview_data* data = ((webview_message_handler*)self)->data;
    LPWSTR source = NULL, current = NULL, wide_json = NULL;
    char* source_utf8 = NULL;
    char* json = NULL;
    size_t source_bytes = 0, json_bytes = 0;
    if (data->state != XUI_WEBVIEW_READY || !data->widget ||
            !data->on_message || !data->message_origin) return S_OK;
    if (FAILED(ICoreWebView2WebMessageReceivedEventArgs_get_Source(args, &source)) ||
            FAILED(ICoreWebView2_get_Source(sender, &current)) ||
            !webview_origin_matches(source, data->message_origin) ||
            !current || wcscmp(source, current) != 0 ||
            FAILED(ICoreWebView2WebMessageReceivedEventArgs_get_WebMessageAsJson(
                args, &wide_json)) ||
            FAILED(webview_utf8_result(source, &source_utf8, &source_bytes)) ||
            FAILED(webview_utf8_result(wide_json, &json, &json_bytes)) ||
            json_bytes > XUI_WEBVIEW_MAX_MESSAGE_BYTES) goto done;
    data->on_message(data->widget, source_utf8, json, json_bytes, data->message_user);
done:
    CoTaskMemFree(source);
    CoTaskMemFree(current);
    CoTaskMemFree(wide_json);
    free(source_utf8);
    free(json);
    return S_OK;
}

static ICoreWebView2WebMessageReceivedEventHandlerVtbl message_vtable = {
    message_query, message_add, message_release, message_invoke
};

static HRESULT STDMETHODCALLTYPE resource_invoke(
    ICoreWebView2WebResourceRequestedEventHandler* self,
    ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args)
{
    xui_webview_data* data = ((webview_resource_handler*)self)->data;
    ICoreWebView2WebResourceRequest* request = NULL;
    ICoreWebView2WebResourceResponse* response = NULL;
    LPWSTR uri = NULL;
    HRESULT hr = S_OK;
    (void)sender;
    if (!data->block_external_resources || data->state != XUI_WEBVIEW_READY)
        return S_OK;
    hr = ICoreWebView2WebResourceRequestedEventArgs_get_Request(args, &request);
    if (SUCCEEDED(hr)) hr = ICoreWebView2WebResourceRequest_get_Uri(request, &uri);
    if (FAILED(hr)) goto done;
    /* Inline srcdoc/data content is still available; every other URI is
     * denied before WebView2 sends a request from the nested HTML frame. */
    if (wcsncmp(uri, L"about:", 6) == 0 ||
        wcsncmp(uri, L"data:", 5) == 0) goto done;
    if (!data->resource_environment) { hr = E_FAIL; goto done; }
    hr = ICoreWebView2Environment_CreateWebResourceResponse(
        data->resource_environment, NULL, 403, L"Forbidden",
        L"Content-Length: 0\r\n", &response);
    if (SUCCEEDED(hr))
        hr = ICoreWebView2WebResourceRequestedEventArgs_put_Response(args,
            response);
    if (SUCCEEDED(hr) && data->blocked_resource_count != UINT32_MAX)
        data->blocked_resource_count++;
done:
    CoTaskMemFree(uri);
    if (request) ICoreWebView2WebResourceRequest_Release(request);
    if (response) ICoreWebView2WebResourceResponse_Release(response);
    return hr;
}

static ICoreWebView2WebResourceRequestedEventHandlerVtbl resource_vtable = {
    resource_query, resource_add, resource_release, resource_invoke
};

static ULONG STDMETHODCALLTYPE script_add(ICoreWebView2ExecuteScriptCompletedHandler* self);

static HRESULT STDMETHODCALLTYPE script_query(ICoreWebView2ExecuteScriptCompletedHandler* self,
    REFIID iid, void** out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (!IsEqualIID(iid, &IID_IUnknown) &&
            !IsEqualIID(iid, &IID_ICoreWebView2ExecuteScriptCompletedHandler))
        return E_NOINTERFACE;
    *out = self;
    script_add(self);
    return S_OK;
}

static ULONG STDMETHODCALLTYPE script_add(ICoreWebView2ExecuteScriptCompletedHandler* self)
{
    return (ULONG)InterlockedIncrement(&((webview_script_handler*)self)->refs);
}

static ULONG STDMETHODCALLTYPE script_release(ICoreWebView2ExecuteScriptCompletedHandler* self)
{
    webview_script_handler* handler = (webview_script_handler*)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (!refs) {
        if (InterlockedExchange(&handler->inflight, 0))
            InterlockedDecrement(&handler->data->inflight_scripts);
        web_request_release(handler->request);
        webview_release(handler->data);
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE script_invoke(ICoreWebView2ExecuteScriptCompletedHandler* self,
    HRESULT error, LPCWSTR result)
{
    webview_script_handler* handler = (webview_script_handler*)self;
    xui_web_request_t* request = handler->request;
    char* json = NULL;
    size_t bytes = 0;
    if (InterlockedExchange(&handler->inflight, 0))
        InterlockedDecrement(&handler->data->inflight_scripts);
    if (request->state != XUI_WEB_REQUEST_PENDING) return S_OK;
    if (handler->data->state != XUI_WEBVIEW_READY ||
            request->epoch != handler->data->page_epoch) {
        web_request_finish(request, XUI_WEB_REQUEST_CANCELLED, E_ABORT, NULL, 0);
    } else if (FAILED(error)) {
        web_request_finish(request, XUI_WEB_REQUEST_FAILED, error, NULL, 0);
    } else {
        error = webview_utf8_result(result, &json, &bytes);
        if (FAILED(error))
            web_request_finish(request, XUI_WEB_REQUEST_FAILED, error, NULL, 0);
        else
            web_request_finish(request, XUI_WEB_REQUEST_COMPLETED, S_OK, json, bytes);
    }
    return S_OK;
}

static ICoreWebView2ExecuteScriptCompletedHandlerVtbl script_vtable = {
    script_query, script_add, script_release, script_invoke
};

static ULONG STDMETHODCALLTYPE capture_add(ICoreWebView2CapturePreviewCompletedHandler* self);

static HRESULT STDMETHODCALLTYPE capture_query(ICoreWebView2CapturePreviewCompletedHandler* self,
    REFIID iid, void** out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (!IsEqualIID(iid, &IID_IUnknown) &&
            !IsEqualIID(iid, &IID_ICoreWebView2CapturePreviewCompletedHandler))
        return E_NOINTERFACE;
    *out = self;
    capture_add(self);
    return S_OK;
}

static ULONG STDMETHODCALLTYPE capture_add(ICoreWebView2CapturePreviewCompletedHandler* self)
{
    return (ULONG)InterlockedIncrement(&((webview_capture_handler*)self)->refs);
}

static ULONG STDMETHODCALLTYPE capture_release(ICoreWebView2CapturePreviewCompletedHandler* self)
{
    webview_capture_handler* handler = (webview_capture_handler*)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (!refs) {
        if (InterlockedExchange(&handler->inflight, 0))
            InterlockedDecrement(&handler->data->inflight_captures);
        IStream_Release(handler->stream);
        web_request_release(handler->request);
        webview_release(handler->data);
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE capture_invoke(ICoreWebView2CapturePreviewCompletedHandler* self,
    HRESULT error)
{
    static const unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    webview_capture_handler* handler = (webview_capture_handler*)self;
    xui_web_request_t* request = handler->request;
    STATSTG stat = {0};
    LARGE_INTEGER zero = {0};
    unsigned char* png = NULL;
    ULONG bytes_read = 0;
    size_t bytes;
    if (InterlockedExchange(&handler->inflight, 0))
        InterlockedDecrement(&handler->data->inflight_captures);
    if (request->state != XUI_WEB_REQUEST_PENDING) return S_OK;
    if (handler->data->state != XUI_WEBVIEW_READY ||
            request->epoch != handler->data->page_epoch) {
        web_request_finish(request, XUI_WEB_REQUEST_CANCELLED, E_ABORT, NULL, 0);
        return S_OK;
    }
    if (FAILED(error)) goto failed;
    error = IStream_Stat(handler->stream, &stat, STATFLAG_NONAME);
    if (FAILED(error)) goto failed;
    if (stat.cbSize.QuadPart < sizeof(signature) ||
            stat.cbSize.QuadPart > XUI_WEBVIEW_MAX_CAPTURE_BYTES) {
        error = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto failed;
    }
    bytes = (size_t)stat.cbSize.QuadPart;
    png = malloc(bytes);
    if (!png) { error = E_OUTOFMEMORY; goto failed; }
    error = IStream_Seek(handler->stream, zero, STREAM_SEEK_SET, NULL);
    if (FAILED(error)) goto failed;
    error = IStream_Read(handler->stream, png, (ULONG)bytes, &bytes_read);
    if (FAILED(error) || bytes_read != (ULONG)bytes ||
            memcmp(png, signature, sizeof(signature)) != 0) {
        error = FAILED(error) ? error : E_FAIL;
        goto failed;
    }
    web_request_finish(request, XUI_WEB_REQUEST_COMPLETED, S_OK, png, bytes);
    return S_OK;
failed:
    free(png);
    web_request_finish(request, XUI_WEB_REQUEST_FAILED, error, NULL, 0);
    return S_OK;
}

static ICoreWebView2CapturePreviewCompletedHandlerVtbl capture_vtable = {
    capture_query, capture_add, capture_release, capture_invoke
};

static HRESULT STDMETHODCALLTYPE navigation_invoke(
    ICoreWebView2NavigationCompletedEventHandler* self, ICoreWebView2* sender,
    ICoreWebView2NavigationCompletedEventArgs* args)
{
    xui_webview_data* data = ((webview_navigation_handler*)self)->data;
    BOOL success = FALSE;
    HRESULT hr;
    (void)sender;
    if (data->state != XUI_WEBVIEW_READY) return S_OK;
    hr = ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(args, &success);
    if (FAILED(hr) || !success) {
        data->error = (int32_t)(FAILED(hr) ? hr : E_FAIL);
        webview_emit(data, XUI_WEBVIEW_EVENT_NAVIGATION_FAILED, data->error);
    } else {
        data->error = 0;
        data->page_loaded = 1;
        webview_emit(data, XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE, 0);
    }
    return S_OK;
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl navigation_vtable = {
    navigation_query, navigation_add, navigation_release, navigation_invoke
};

static HRESULT webview_register_focus_handlers(xui_webview_data* data)
{
    webview_focus_handler* focus;
    webview_move_focus_handler* move;
    HRESULT hr;
    focus = calloc(1, sizeof(*focus));
    if (!focus) return E_OUTOFMEMORY;
    focus->iface.lpVtbl = &focus_vtable;
    focus->refs = 1;
    focus->data = data;
    focus->gained = 1;
    webview_retain(data);
    hr = ICoreWebView2Controller_add_GotFocus(data->controller, &focus->iface,
        &data->focus_gained_token);
    focus_release(&focus->iface);
    if (FAILED(hr)) return hr;
    data->focus_gained_registered = 1;

    focus = calloc(1, sizeof(*focus));
    if (!focus) return E_OUTOFMEMORY;
    focus->iface.lpVtbl = &focus_vtable;
    focus->refs = 1;
    focus->data = data;
    webview_retain(data);
    hr = ICoreWebView2Controller_add_LostFocus(data->controller, &focus->iface,
        &data->focus_lost_token);
    focus_release(&focus->iface);
    if (FAILED(hr)) return hr;
    data->focus_lost_registered = 1;

    move = calloc(1, sizeof(*move));
    if (!move) return E_OUTOFMEMORY;
    move->iface.lpVtbl = &move_focus_vtable;
    move->refs = 1;
    move->data = data;
    webview_retain(data);
    hr = ICoreWebView2Controller_add_MoveFocusRequested(data->controller, &move->iface,
        &data->move_focus_token);
    move_focus_release(&move->iface);
    if (FAILED(hr)) return hr;
    data->move_focus_registered = 1;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE controller_invoke(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* self,
    HRESULT error, ICoreWebView2Controller* controller_result)
{
    xui_webview_data* data = ((webview_controller_handler*)self)->data;
    webview_navigation_handler* navigation;
    webview_navigation_starting_handler* starting;
    webview_process_handler* process;
    webview_message_handler* message;
    ICoreWebView2Settings* settings = NULL;
    HRESULT hr;
    if (data->state != XUI_WEBVIEW_INITIALIZING) {
        if (controller_result) ICoreWebView2Controller_Close(controller_result);
        return S_OK;
    }
    if (FAILED(error) || !controller_result) {
        webview_failed(data, FAILED(error) ? error : E_FAIL);
        return S_OK;
    }
    data->controller = controller_result;
    ICoreWebView2Controller_AddRef(data->controller);
    hr = ICoreWebView2Controller_get_CoreWebView2(data->controller, &data->view);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    hr = ICoreWebView2Controller_put_IsVisible(data->controller, FALSE);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->controller_visible = 0;
    navigation = calloc(1, sizeof(*navigation));
    if (!navigation) { webview_failed(data, E_OUTOFMEMORY); return S_OK; }
    navigation->iface.lpVtbl = &navigation_vtable;
    navigation->refs = 1;
    navigation->data = data;
    webview_retain(data);
    hr = ICoreWebView2_add_NavigationCompleted(data->view, &navigation->iface,
        &data->navigation_token);
    navigation_release(&navigation->iface);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->navigation_registered = 1;
    starting = calloc(1, sizeof(*starting));
    if (!starting) { webview_failed(data, E_OUTOFMEMORY); return S_OK; }
    starting->iface.lpVtbl = &navigation_starting_vtable;
    starting->refs = 1;
    starting->data = data;
    webview_retain(data);
    hr = ICoreWebView2_add_NavigationStarting(data->view, &starting->iface,
        &data->navigation_starting_token);
    navigation_starting_release(&starting->iface);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->navigation_starting_registered = 1;
    process = calloc(1, sizeof(*process));
    if (!process) { webview_failed(data, E_OUTOFMEMORY); return S_OK; }
    process->iface.lpVtbl = &process_vtable;
    process->refs = 1;
    process->data = data;
    webview_retain(data);
    hr = ICoreWebView2_add_ProcessFailed(data->view, &process->iface,
        &data->process_token);
    process_release(&process->iface);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->process_registered = 1;
    hr = ICoreWebView2_get_Settings(data->view, &settings);
    if (SUCCEEDED(hr)) hr = ICoreWebView2Settings_put_IsWebMessageEnabled(settings, FALSE);
    if (settings) ICoreWebView2Settings_Release(settings);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    message = calloc(1, sizeof(*message));
    if (!message) { webview_failed(data, E_OUTOFMEMORY); return S_OK; }
    message->iface.lpVtbl = &message_vtable;
    message->refs = 1;
    message->data = data;
    webview_retain(data);
    hr = ICoreWebView2_add_WebMessageReceived(data->view, &message->iface,
        &data->message_token);
    message_release(&message->iface);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->message_registered = 1;
    hr = webview_register_focus_handlers(data);
    if (FAILED(hr)) { webview_failed(data, hr); return S_OK; }
    data->state = XUI_WEBVIEW_READY;
    webview_sync_host(data);
    webview_emit(data, XUI_WEBVIEW_EVENT_READY, 0);
    return S_OK;
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl controller_vtable = {
    controller_query, controller_add, controller_release, controller_invoke
};

static HRESULT STDMETHODCALLTYPE environment_invoke(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* self,
    HRESULT error, ICoreWebView2Environment* environment_result)
{
    xui_webview_data* data = ((webview_environment_handler*)self)->data;
    webview_controller_handler* controller;
    HRESULT hr;
    if (data->state != XUI_WEBVIEW_INITIALIZING) return S_OK;
    if (FAILED(error) || !environment_result) {
        webview_failed(data, FAILED(error) ? error : E_FAIL);
        return S_OK;
    }
    controller = calloc(1, sizeof(*controller));
    if (!controller) { webview_failed(data, E_OUTOFMEMORY); return S_OK; }
    controller->iface.lpVtbl = &controller_vtable;
    controller->refs = 1;
    controller->data = data;
    webview_retain(data);
    hr = ICoreWebView2Environment_CreateCoreWebView2Controller(
        environment_result, data->host, &controller->iface);
    controller_release(&controller->iface);
    if (FAILED(hr)) webview_failed(data, hr);
    return S_OK;
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl environment_vtable = {
    environment_query, environment_add, environment_release, environment_invoke
};

static int webview_start(xui_webview_data* data)
{
    webview_environment_handler* environment;
    LPWSTR version = NULL;
    wchar_t* folder = NULL;
    HRESULT hr;
    webview_retain(data);
    data->thread = GetCurrentThreadId();
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) goto failed;
    data->com_initialized = 1;
    data->parent = (HWND)xgePlatformNativeHandle();
    if (!data->parent || !IsWindow(data->parent)) { hr = E_HANDLE; goto failed; }
    if (GetWindowThreadProcessId(data->parent, NULL) != data->thread) {
        hr = RPC_E_WRONG_THREAD;
        goto failed;
    }
    hr = GetAvailableCoreWebView2BrowserVersionString(NULL, &version);
    if (FAILED(hr)) goto failed;
    CoTaskMemFree(version); version = NULL;
    if (data->folder) {
        folder = webview_utf16(data->folder, strlen(data->folder));
        if (!folder) { hr = E_INVALIDARG; goto failed; }
    }
    data->host = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_CLIPCHILDREN |
        WS_CLIPSIBLINGS, 0, 0, 1, 1, data->parent, NULL, GetModuleHandleW(NULL), NULL);
    if (!data->host) { hr = HRESULT_FROM_WIN32(GetLastError()); goto failed; }
    webview_sync_host(data);
    environment = calloc(1, sizeof(*environment));
    if (!environment) { hr = E_OUTOFMEMORY; goto failed; }
    environment->iface.lpVtbl = &environment_vtable;
    environment->refs = 1;
    environment->data = data;
    webview_retain(data);
    hr = CreateCoreWebView2EnvironmentWithOptions(NULL, folder, NULL, &environment->iface);
    environment_release(&environment->iface);
    free(folder); folder = NULL;
    if (FAILED(hr)) goto failed;
    webview_release(data);
    return XUI_OK;
failed:
    if (version) CoTaskMemFree(version);
    free(folder);
    webview_failed(data, hr);
    webview_release(data);
    return XUI_ERROR_BACKEND_FAILED;
}
#else
static void webview_release(xui_webview_data* data)
{
    free(data->folder);
    free(data);
}
#endif

static int webview_widget_event(xui_widget widget, const xui_event_t* event, void* user)
{
    xui_webview_data* data = webview_get(widget);
    (void)user;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    if (!data || !event || data->state != XUI_WEBVIEW_READY ||
            !webview_on_owner_thread(data)) return XUI_OK;
    if (event->iType == XUI_EVENT_FOCUS && data->controller && data->host_visible &&
            xuiWidgetGetEffectiveEnabled(widget) &&
            xuiGetFocusWidget(xuiWidgetGetContext(widget)) == widget) {
        HWND focused = GetFocus();
        if (!focused || (focused != data->host && !IsChild(data->host, focused))) {
            webview_retain(data);
            (void)ICoreWebView2Controller_MoveFocus(data->controller,
                COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
            webview_release(data);
        }
    } else if (event->iType == XUI_EVENT_BLUR &&
            xuiGetFocusWidget(xuiWidgetGetContext(widget)) != widget && data->host &&
            data->parent && IsWindow(data->parent)) {
        HWND focused = GetFocus();
        if (focused && (focused == data->host || IsChild(data->host, focused)))
            SetFocus(data->parent);
    }
#else
    (void)data; (void)event;
#endif
    return XUI_OK;
}

static int webview_init(xui_widget widget, void* type_data, const void* create, void* user)
{
    const xui_webview_desc_t* desc = create;
    xui_webview_data** slot = type_data;
    xui_webview_data* data;
    (void)user;
    if (!desc || desc->iSize != sizeof(*desc)) return XUI_ERROR_INVALID_ARGUMENT;
    data = calloc(1, sizeof(*data));
    if (!data) return XUI_ERROR_OUT_OF_MEMORY;
    data->widget = widget;
    data->on_event = desc->onEvent;
    data->user = desc->pUser;
    data->state = XUI_WEBVIEW_CREATED;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    data->refs = 1;
    data->page_epoch = 1;
#endif
    if (desc->sUserDataFolder) {
        data->folder = webview_copy_string(desc->sUserDataFolder);
        if (!data->folder) { webview_release(data); return XUI_ERROR_OUT_OF_MEMORY; }
    }
    *slot = data;
    if (xuiWidgetSetFocusable(widget, 1) != XUI_OK ||
            xuiWidgetSetTabStop(widget, 1) != XUI_OK ||
            xuiWidgetSetEventCallback(widget, webview_widget_event, NULL) != XUI_OK) {
        *slot = NULL;
        webview_release(data);
        return XUI_ERROR_INVALID_STATE;
    }
    return XUI_OK;
}

static void webview_destroy(xui_widget widget, void* type_data, void* user)
{
    xui_webview_data** slot = type_data;
    xui_webview_data* data = *slot;
    (void)widget; (void)user;
    if (!data) return;
    *slot = NULL;
    data->widget = NULL;
    data->state = XUI_WEBVIEW_CLOSED;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    webview_stop_backend(data);
#endif
    webview_release(data);
}

static int webview_update(xui_widget widget, float delta, void* user)
{
    xui_webview_data* data = webview_get(widget);
    (void)delta; (void)user;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    if (data && data->process_failed_pending) {
        webview_retain(data);
        data->process_failed_pending = 0;
        if (data->state == XUI_WEBVIEW_FAILED) {
            if (data->widget &&
                xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) == data->widget)
                (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), NULL);
            webview_stop_backend(data);
            webview_emit(data, XUI_WEBVIEW_EVENT_FAILED, data->error);
        }
        webview_release(data);
    } else if (data && data->state != XUI_WEBVIEW_CLOSED) {
        webview_retain(data);
        webview_sync_host(data);
        webview_release(data);
    }
#else
    (void)data;
#endif
    return XUI_OK;
}

XUI_API xui_widget_type xuiWebViewGetType(xui_context context)
{
    xui_widget_type type = xuiWidgetFindType(context, "webview");
    xui_widget_type_desc_t desc = {0};
    if (type) return type;
    desc.iSize = sizeof(desc);
    desc.sName = "webview";
    desc.pParent = xuiWidgetGetBaseType();
    desc.iTypeDataSize = sizeof(xui_webview_data*);
    desc.onInit = webview_init;
    desc.onDestroy = webview_destroy;
    desc.onUpdate = webview_update;
    desc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY |
        XUI_WIDGET_TYPE_UPDATE_ON_INACTIVE;
    desc.tLayout.iLayoutType = XUI_LAYOUT_MANUAL;
    desc.tLayout.iWidthMode = XUI_SIZE_FILL;
    desc.tLayout.iHeightMode = XUI_SIZE_FIXED;
    desc.tLayout.fPreferredHeight = 240;
    desc.tLayout.iFlowMode = XUI_FLOW_BLOCK;
    desc.tLayout.iOverflow = XUI_OVERFLOW_HIDDEN;
    desc.tLayout.fMaxWidth = desc.tLayout.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
    desc.tLayout.fShrink = 1;
    desc.tLayout.iTableRowSpan = desc.tLayout.iTableColumnSpan = desc.tLayout.iGridColumnCount = 1;
    desc.tCachePolicy.iSize = sizeof(desc.tCachePolicy);
    desc.tCachePolicy.iPolicy = XUI_CACHE_POLICY_SELF;
    return xuiWidgetRegisterType(context, &type, &desc) == XUI_OK ? type : NULL;
}

XUI_API int xuiWebViewCreate(xui_context context, xui_widget* out, const xui_webview_desc_t* desc)
{
    xui_widget_type type;
    if (out) *out = NULL;
    if (!context || !out || !desc || desc->iSize != sizeof(*desc))
        return XUI_ERROR_INVALID_ARGUMENT;
    type = xuiWebViewGetType(context);
    return type ? xuiWidgetCreateTyped(context, type, out, desc) : XUI_ERROR_OUT_OF_MEMORY;
}

XUI_API int xuiWebViewInitializeAsync(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    if (!data) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_CREATED) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    data->state = XUI_WEBVIEW_INITIALIZING;
    return webview_start(data);
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewFocus(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    if (!data) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY ||
            !xuiWidgetIsAttachedToContext(widget) ||
            !xuiWidgetGetEffectiveVisible(widget) ||
            !xuiWidgetGetEffectiveEnabled(widget))
        return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    int result;
    HRESULT hr;
    if (!webview_on_owner_thread(data) || !data->controller || !data->host_visible)
        return XUI_ERROR_INVALID_STATE;
    webview_retain(data);
    result = xuiSetFocusWidget(xuiWidgetGetContext(widget), widget);
    if (result != XUI_OK || data->state != XUI_WEBVIEW_READY || !data->controller) {
        webview_release(data);
        return result != XUI_OK ? result : XUI_ERROR_INVALID_STATE;
    }
    hr = ICoreWebView2Controller_MoveFocus(data->controller,
        COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
    if (FAILED(hr) && data->widget &&
            xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) == data->widget)
        (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), NULL);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    webview_release(data);
    return SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewNavigate(xui_widget widget, const char* url)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || !url) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    wchar_t* wide;
    HRESULT hr;
    if (!webview_on_owner_thread(data)) return XUI_ERROR_INVALID_STATE;
    wide = webview_utf16(url, strlen(url));
    if (!wide) return XUI_ERROR_INVALID_ARGUMENT;
    webview_retain(data);
    webview_advance_page(data);
    hr = ICoreWebView2_Navigate(data->view, wide);
    free(wide);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    {
        int result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
            SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
        webview_release(data);
        return result;
    }
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewLoadHtml(xui_widget widget, const char* html, size_t bytes)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || !html) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    wchar_t* wide;
    HRESULT hr;
    if (!webview_on_owner_thread(data)) return XUI_ERROR_INVALID_STATE;
    wide = webview_utf16(html, bytes);
    if (!wide) return XUI_ERROR_INVALID_ARGUMENT;
    webview_retain(data);
    webview_advance_page(data);
    hr = ICoreWebView2_NavigateToString(data->view, wide);
    free(wide);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    {
        int result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
            SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
        webview_release(data);
        return result;
    }
#else
    (void)bytes;
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewSetZoomFactor(xui_widget widget, double factor)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || factor != factor || factor < 0.25 || factor > 5.0)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    HRESULT hr;
    if (!webview_on_owner_thread(data) || !data->controller)
        return XUI_ERROR_INVALID_STATE;
    webview_retain(data);
    hr = ICoreWebView2Controller_put_ZoomFactor(data->controller, factor);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    webview_release(data);
    return SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewGetZoomFactor(xui_widget widget, double* out)
{
    xui_webview_data* data = webview_get(widget);
    if (out) *out = 0;
    if (!data || !out) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    HRESULT hr;
    if (!webview_on_owner_thread(data) || !data->controller)
        return XUI_ERROR_INVALID_STATE;
    webview_retain(data);
    hr = ICoreWebView2Controller_get_ZoomFactor(data->controller, out);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    webview_release(data);
    return SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewMapLocalFolder(xui_widget widget, const char* host,
    const char* folder, int access)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || !host || !folder || access < XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN ||
            access > XUI_WEB_LOCAL_ACCESS_ALL) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    ICoreWebView2_3* view3 = NULL;
    COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND native_access;
    wchar_t* wide_host;
    wchar_t* wide_folder;
    HRESULT hr;
    int result;
    if (!webview_on_owner_thread(data) || !data->view)
        return XUI_ERROR_INVALID_STATE;
    if (!webview_valid_virtual_host(host)) return XUI_ERROR_INVALID_ARGUMENT;
    wide_host = webview_utf16(host, strlen(host));
    wide_folder = webview_absolute_folder(folder);
    if (!wide_host || !wide_folder) {
        free(wide_host);
        free(wide_folder);
        return XUI_ERROR_INVALID_ARGUMENT;
    }
    native_access = access == XUI_WEB_LOCAL_ACCESS_ALL ?
        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW :
        access == XUI_WEB_LOCAL_ACCESS_SUBRESOURCE ?
        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS :
        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY;
    webview_retain(data);
    hr = ICoreWebView2_QueryInterface(data->view, &IID_ICoreWebView2_3, (void**)&view3);
    if (SUCCEEDED(hr)) {
        hr = ICoreWebView2_3_SetVirtualHostNameToFolderMapping(view3,
            wide_host, wide_folder, native_access);
        ICoreWebView2_3_Release(view3);
    }
    free(wide_host);
    free(wide_folder);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
        SUCCEEDED(hr) ? XUI_OK : hr == E_NOINTERFACE ? XUI_ERROR_UNSUPPORTED :
        XUI_ERROR_BACKEND_FAILED;
    webview_release(data);
    return result;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API int xuiWebViewUnmapLocalFolder(xui_widget widget, const char* host)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || !host) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    ICoreWebView2_3* view3 = NULL;
    wchar_t* wide_host;
    HRESULT hr;
    int result;
    if (!webview_on_owner_thread(data) || !data->view)
        return XUI_ERROR_INVALID_STATE;
    if (!webview_valid_virtual_host(host)) return XUI_ERROR_INVALID_ARGUMENT;
    wide_host = webview_utf16(host, strlen(host));
    if (!wide_host) return XUI_ERROR_INVALID_ARGUMENT;
    webview_retain(data);
    hr = ICoreWebView2_QueryInterface(data->view, &IID_ICoreWebView2_3, (void**)&view3);
    if (SUCCEEDED(hr)) {
        hr = ICoreWebView2_3_ClearVirtualHostNameToFolderMapping(view3, wide_host);
        ICoreWebView2_3_Release(view3);
    }
    free(wide_host);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
        SUCCEEDED(hr) ? XUI_OK : hr == E_NOINTERFACE ? XUI_ERROR_UNSUPPORTED :
        XUI_ERROR_BACKEND_FAILED;
    webview_release(data);
    return result;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API int xuiWebViewSetMessageHandler(xui_widget widget, const char* origin,
    xui_webview_message_proc callback, void* user)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || (!!origin != !!callback)) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    wchar_t* wide = NULL;
    ICoreWebView2Settings* settings = NULL;
    HRESULT hr;
    int result;
    if (!webview_on_owner_thread(data) || !data->view)
        return XUI_ERROR_INVALID_STATE;
    if (callback) {
        if (!webview_valid_message_origin(origin)) return XUI_ERROR_INVALID_ARGUMENT;
        wide = webview_utf16(origin, strlen(origin));
        if (!wide) return XUI_ERROR_OUT_OF_MEMORY;
    }
    webview_retain(data);
    hr = ICoreWebView2_get_Settings(data->view, &settings);
    if (SUCCEEDED(hr))
        hr = ICoreWebView2Settings_put_IsWebMessageEnabled(settings, callback != NULL);
    if (settings) ICoreWebView2Settings_Release(settings);
    result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
        SUCCEEDED(hr) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
    if (result == XUI_OK) {
        free(data->message_origin);
        data->message_origin = wide;
        data->on_message = callback;
        data->message_user = user;
        wide = NULL;
    }
    free(wide);
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    webview_release(data);
    return result;
#else
    (void)user;
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API int xuiWebViewBlockExternalResources(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    if (!data) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    ICoreWebView2_22* view22 = NULL;
    ICoreWebView2_2* view2 = NULL;
    ICoreWebView2Environment* environment = NULL;
    webview_resource_handler* handler = NULL;
    HRESULT hr;
    int result;
    if (!webview_on_owner_thread(data) || !data->view)
        return XUI_ERROR_INVALID_STATE;
    if (data->block_external_resources) return XUI_OK;
    webview_retain(data);
    hr = ICoreWebView2_QueryInterface(data->view, &IID_ICoreWebView2_22,
        (void**)&view22);
    if (SUCCEEDED(hr))
        hr = ICoreWebView2_QueryInterface(data->view, &IID_ICoreWebView2_2,
            (void**)&view2);
    if (SUCCEEDED(hr)) hr = ICoreWebView2_2_get_Environment(view2, &environment);
    if (FAILED(hr)) goto done;
    handler = calloc(1, sizeof(*handler));
    if (!handler) { hr = E_OUTOFMEMORY; goto done; }
    handler->iface.lpVtbl = &resource_vtable;
    handler->refs = 1;
    handler->data = data;
    webview_retain(data);
    hr = ICoreWebView2_add_WebResourceRequested(data->view, &handler->iface,
        &data->resource_token);
    resource_release(&handler->iface);
    if (FAILED(hr)) goto done;
    data->resource_registered = 1;
    hr = ICoreWebView2_22_AddWebResourceRequestedFilterWithRequestSourceKinds(
        view22, L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL,
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL);
    if (FAILED(hr)) {
        ICoreWebView2_remove_WebResourceRequested(data->view,
            data->resource_token);
        data->resource_registered = 0;
        goto done;
    }
    data->resource_environment = environment;
    environment = NULL;
    data->block_external_resources = 1;
done:
    if (environment) ICoreWebView2Environment_Release(environment);
    if (view2) ICoreWebView2_2_Release(view2);
    if (view22) ICoreWebView2_22_Release(view22);
    result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
        SUCCEEDED(hr) ? XUI_OK : hr == E_NOINTERFACE ? XUI_ERROR_UNSUPPORTED :
        hr == E_OUTOFMEMORY ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR_BACKEND_FAILED;
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    webview_release(data);
    return result;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API uint32_t xuiWebViewGetBlockedResourceCount(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    return data && webview_on_owner_thread(data) ? data->blocked_resource_count : 0;
#else
    (void)data;
    return 0;
#endif
}

XUI_WEBVIEW_INTERNAL_API int xuiWebViewPostMessageJson(xui_widget widget, const char* json, size_t bytes)
{
    xui_webview_data* data = webview_get(widget);
    if (!data || !json || !bytes || bytes > XUI_WEBVIEW_MAX_MESSAGE_BYTES)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    LPWSTR source = NULL;
    wchar_t* wide;
    HRESULT hr;
    int result;
    if (!webview_on_owner_thread(data) || !data->view || !data->on_message ||
            !data->message_origin) return XUI_ERROR_INVALID_STATE;
    wide = webview_utf16(json, bytes);
    if (!wide) return XUI_ERROR_INVALID_ARGUMENT;
    webview_retain(data);
    hr = ICoreWebView2_get_Source(data->view, &source);
    if (FAILED(hr) || !webview_origin_matches(source, data->message_origin)) {
        result = FAILED(hr) ? XUI_ERROR_BACKEND_FAILED : XUI_ERROR_INVALID_STATE;
    } else {
        hr = ICoreWebView2_PostWebMessageAsJson(data->view, wide);
        result = data->state != XUI_WEBVIEW_READY ? XUI_ERROR_INVALID_STATE :
            SUCCEEDED(hr) ? XUI_OK : hr == E_INVALIDARG ?
            XUI_ERROR_INVALID_ARGUMENT : XUI_ERROR_BACKEND_FAILED;
    }
    if (data->state == XUI_WEBVIEW_READY) data->error = (int32_t)hr;
    CoTaskMemFree(source);
    free(wide);
    webview_release(data);
    return result;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API int xuiWebViewEvalScriptAsync(xui_widget widget, const char* script,
    size_t bytes, xui_web_request* out)
{
    xui_webview_data* data = webview_get(widget);
    if (out) *out = NULL;
    if (!data || !script || !out) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    xui_web_request_t* request;
    webview_script_handler* handler;
    wchar_t* wide;
    HRESULT hr;
    if (!webview_on_owner_thread(data) || !data->view)
        return XUI_ERROR_INVALID_STATE;
    if (InterlockedCompareExchange(&data->inflight_scripts, 0, 0) >=
            (LONG)XUI_WEBVIEW_MAX_PENDING_SCRIPTS)
        return XUI_ERROR_LIMIT_EXCEEDED;
    if (bytes > XUI_WEBVIEW_MAX_SCRIPT_BYTES) return XUI_ERROR_INVALID_ARGUMENT;
    wide = webview_utf16(script, bytes);
    if (!wide) return XUI_ERROR_INVALID_ARGUMENT;
    request = calloc(1, sizeof(*request));
    handler = calloc(1, sizeof(*handler));
    if (!request || !handler) {
        free(request);
        free(handler);
        free(wide);
        return XUI_ERROR_OUT_OF_MEMORY;
    }
    request->refs = 1;
    request->kind = WEB_REQUEST_SCRIPT;
    request->state = XUI_WEB_REQUEST_PENDING;
    request->thread = GetCurrentThreadId();
    request->epoch = data->page_epoch;
    request->owner = data;
    request->next = data->requests;
    data->requests = request;
    handler->iface.lpVtbl = &script_vtable;
    handler->refs = 1;
    handler->inflight = 1;
    handler->data = data;
    handler->request = request;
    webview_retain(data);
    web_request_retain(request);
    InterlockedIncrement(&data->inflight_scripts);
    hr = ICoreWebView2_ExecuteScript(data->view, wide, &handler->iface);
    free(wide);
    if (FAILED(hr)) {
        web_request_finish(request, XUI_WEB_REQUEST_FAILED, hr, NULL, 0);
        script_release(&handler->iface);
        web_request_release(request);
        return XUI_ERROR_BACKEND_FAILED;
    }
    *out = request;
    script_release(&handler->iface);
    return XUI_OK;
#else
    (void)bytes;
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API int xuiWebViewCapturePngAsync(xui_widget widget, xui_web_request* out)
{
    xui_webview_data* data = webview_get(widget);
    if (out) *out = NULL;
    if (!data || !out) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state != XUI_WEBVIEW_READY) return XUI_ERROR_INVALID_STATE;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    xui_web_request_t* request;
    webview_capture_handler* handler;
    IStream* stream = NULL;
    LONG width, height;
    HRESULT hr;
    if (!webview_on_owner_thread(data) || !data->view || !data->controller ||
            !data->host_visible || !data->controller_visible ||
            !data->page_loaded || !data->controller_bounds_valid)
        return XUI_ERROR_INVALID_STATE;
    width = data->controller_bounds.right - data->controller_bounds.left;
    height = data->controller_bounds.bottom - data->controller_bounds.top;
    if (width <= 0 || height <= 0 || width > XUI_WEBVIEW_MAX_CAPTURE_EDGE ||
            height > XUI_WEBVIEW_MAX_CAPTURE_EDGE ||
            (uint64_t)width * (uint64_t)height > 16000000u)
        return XUI_ERROR_LIMIT_EXCEEDED;
    if (InterlockedCompareExchange(&data->inflight_captures, 0, 0) >=
            (LONG)XUI_WEBVIEW_MAX_PENDING_CAPTURES)
        return XUI_ERROR_LIMIT_EXCEEDED;
    hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
    if (FAILED(hr)) return XUI_ERROR_OUT_OF_MEMORY;
    request = calloc(1, sizeof(*request));
    handler = calloc(1, sizeof(*handler));
    if (!request || !handler) {
        free(request); free(handler);
        IStream_Release(stream);
        return XUI_ERROR_OUT_OF_MEMORY;
    }
    request->refs = 1;
    request->kind = WEB_REQUEST_PNG;
    request->state = XUI_WEB_REQUEST_PENDING;
    request->thread = GetCurrentThreadId();
    request->epoch = data->page_epoch;
    request->owner = data;
    request->next = data->requests;
    data->requests = request;
    handler->iface.lpVtbl = &capture_vtable;
    handler->refs = 1;
    handler->inflight = 1;
    handler->data = data;
    handler->request = request;
    handler->stream = stream;
    webview_retain(data);
    web_request_retain(request);
    InterlockedIncrement(&data->inflight_captures);
    hr = ICoreWebView2_CapturePreview(data->view,
        COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG, stream, &handler->iface);
    if (FAILED(hr)) {
        web_request_finish(request, XUI_WEB_REQUEST_FAILED, hr, NULL, 0);
        capture_release(&handler->iface);
        web_request_release(request);
        return XUI_ERROR_BACKEND_FAILED;
    }
    *out = request;
    capture_release(&handler->iface);
    return XUI_OK;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_WEBVIEW_INTERNAL_API void xuiWebRequestRetain(xui_web_request request)
{
    web_request_retain(request);
}

XUI_WEBVIEW_INTERNAL_API void xuiWebRequestRelease(xui_web_request request)
{
    web_request_release(request);
}

XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetState(xui_web_request request)
{
    return request ? request->state : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_WEBVIEW_INTERNAL_API int32_t xuiWebRequestGetNativeError(xui_web_request request)
{
    return request ? request->error : 0;
}

XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetResultJson(xui_web_request request,
    const char** json, size_t* bytes)
{
    if (json) *json = NULL;
    if (bytes) *bytes = 0;
    if (!request || !json || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (request->kind != WEB_REQUEST_SCRIPT ||
            request->state != XUI_WEB_REQUEST_COMPLETED)
        return XUI_ERROR_INVALID_STATE;
    *json = request->result;
    *bytes = request->bytes;
    return XUI_OK;
}

XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetResultPng(xui_web_request request,
    const void** png, size_t* bytes)
{
    if (png) *png = NULL;
    if (bytes) *bytes = 0;
    if (!request || !png || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (request->kind != WEB_REQUEST_PNG ||
            request->state != XUI_WEB_REQUEST_COMPLETED)
        return XUI_ERROR_INVALID_STATE;
    *png = request->result;
    *bytes = request->bytes;
    return XUI_OK;
}

XUI_WEBVIEW_INTERNAL_API int xuiWebRequestCancel(xui_web_request request)
{
    if (!request) return XUI_ERROR_INVALID_ARGUMENT;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    if (request->thread != GetCurrentThreadId()) return XUI_ERROR_INVALID_STATE;
    if (request->state == XUI_WEB_REQUEST_PENDING)
        web_request_finish(request, XUI_WEB_REQUEST_CANCELLED, E_ABORT, NULL, 0);
#endif
    return XUI_OK;
}

XUI_API int xuiWebViewGetState(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    return data ? data->state : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int32_t xuiWebViewGetNativeError(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    return data ? data->error : 0;
}

#if defined(XUI_WEBVIEW_TEST_EXPORTS)
XUI_WEBVIEW_INTERNAL_API int xuiWebViewTestBrowserProcessId(
    xui_widget widget, uint32_t* out)
{
    xui_webview_data* data = webview_get(widget);
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = 0;
    if (!data) return XUI_ERROR_INVALID_ARGUMENT;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    UINT32 pid = 0;
    if (data->state != XUI_WEBVIEW_READY || !data->view ||
        !webview_on_owner_thread(data)) return XUI_ERROR_INVALID_STATE;
    if (FAILED(ICoreWebView2_get_BrowserProcessId(data->view, &pid)) || !pid)
        return XUI_ERROR_BACKEND_FAILED;
    *out = pid;
    return XUI_OK;
#else
    return XUI_ERROR_UNSUPPORTED;
#endif
}
#endif

XUI_API int xuiWebViewClose(xui_widget widget)
{
    xui_webview_data* data = webview_get(widget);
    if (!data) return XUI_ERROR_INVALID_ARGUMENT;
    if (data->state == XUI_WEBVIEW_CLOSED) return XUI_OK;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    if (!webview_on_owner_thread(data)) return XUI_ERROR_INVALID_STATE;
    webview_retain(data);
#endif
    data->state = XUI_WEBVIEW_CLOSED;
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    if (data->widget && xuiGetFocusWidget(xuiWidgetGetContext(data->widget)) == data->widget)
        (void)xuiSetFocusWidget(xuiWidgetGetContext(data->widget), NULL);
    webview_stop_backend(data);
#endif
    webview_emit(data, XUI_WEBVIEW_EVENT_CLOSED, 0);
#if defined(_WIN32) && defined(XUI_ENABLE_WEBVIEW2)
    webview_release(data);
#endif
    return XUI_OK;
}

#endif
