#include "../xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_native_accessibility_internal.h"
#include "xui_internal.h"
#if defined(_WIN32)
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <commctrl.h>
#include <oleauto.h>
#include <uiautomation.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/* All queries and actions belong to the owning UI STA. UIA marshals its
 * calls there (UseComThreading); no worker may access an XUI widget. The
 * cache owns a reference, so a client's Release never unlinks a live node. */
typedef struct native_bridge native_bridge;
typedef struct native_node native_node;
struct native_node {
    IRawElementProviderSimple simple;
    IRawElementProviderFragment fragment;
    IRawElementProviderFragmentRoot root;
    IValueProvider value;
    IInvokeProvider invoke;
    IToggleProvider toggle;
    LONG refs;
    native_bridge* bridge;
    xui_widget widget;
    uint64_t id;
    LONG serial;
    int host;
    native_node* next;
};
struct native_bridge {
    LONG refs;
    DWORD thread;
    HWND window;
    xui_context context;
    xui_rect_t window_rect;
    native_node* nodes;
    native_node* host;
    native_bridge* next;
};
static native_bridge* bridges;
static DWORD owner_thread;
static LONG next_serial;
static LRESULT (WINAPI *uia_return)(HWND, WPARAM, LPARAM, IRawElementProviderSimple*);
static HRESULT (WINAPI *uia_host)(HWND, IRawElementProviderSimple**);
static HRESULT (WINAPI *uia_event)(IRawElementProviderSimple*, EVENTID);
static HRESULT (WINAPI *uia_structure)(IRawElementProviderSimple*, enum StructureChangeType, int*, int);
static HRESULT (WINAPI *uia_disconnect)(IRawElementProviderSimple*);
static BOOL (WINAPI *uia_listening)(void);
static BOOL (WINAPI *subclass_set)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR);
static BOOL (WINAPI *subclass_remove)(HWND, SUBCLASSPROC, UINT_PTR);
static LRESULT (WINAPI *subclass_default)(HWND, UINT, WPARAM, LPARAM);
static IRawElementProviderSimpleVtbl simple_vtbl;
static IRawElementProviderFragmentVtbl fragment_vtbl;
static IRawElementProviderFragmentRootVtbl root_vtbl;
static IValueProviderVtbl value_vtbl;
static IInvokeProviderVtbl invoke_vtbl;
static IToggleProviderVtbl toggle_vtbl;

#define NODE(type, member, p) ((native_node*)((char*)(p) - offsetof(native_node, member)))
static ULONG native_addref(native_node* n) { return (ULONG)InterlockedIncrement(&n->refs); }
static void bridge_release(native_bridge* b) { if (!InterlockedDecrement(&b->refs)) free(b); }
static ULONG native_release(native_node* n)
{
    ULONG refs = (ULONG)InterlockedDecrement(&n->refs);
    if (!refs) { bridge_release(n->bridge); free(n); }
    return refs;
}
static HRESULT native_query(native_node* n, REFIID iid, void** out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_IRawElementProviderSimple)) *out = &n->simple;
    else if (IsEqualIID(iid, &IID_IRawElementProviderFragment)) *out = &n->fragment;
    else if (n->host && IsEqualIID(iid, &IID_IRawElementProviderFragmentRoot)) *out = &n->root;
    else if (IsEqualIID(iid, &IID_IValueProvider)) *out = &n->value;
    else if (IsEqualIID(iid, &IID_IInvokeProvider)) *out = &n->invoke;
    else if (IsEqualIID(iid, &IID_IToggleProvider)) *out = &n->toggle;
    else return E_NOINTERFACE;
    native_addref(n); return S_OK;
}
#define UNKNOWN_METHODS(type, member, prefix) \
static HRESULT STDMETHODCALLTYPE prefix##_query(type* p, REFIID i, void** o) { return native_query(NODE(type, member, p), i, o); } \
static ULONG STDMETHODCALLTYPE prefix##_addref(type* p) { return native_addref(NODE(type, member, p)); } \
static ULONG STDMETHODCALLTYPE prefix##_release(type* p) { return native_release(NODE(type, member, p)); }
UNKNOWN_METHODS(IRawElementProviderSimple, simple, simple)
UNKNOWN_METHODS(IRawElementProviderFragment, fragment, fragment)
UNKNOWN_METHODS(IRawElementProviderFragmentRoot, root, root)
UNKNOWN_METHODS(IValueProvider, value, value)
UNKNOWN_METHODS(IInvokeProvider, invoke, invoke)
UNKNOWN_METHODS(IToggleProvider, toggle, toggle)

static HRESULT native_begin(native_node* n, xui_context* context)
{
    native_bridge* b = n->bridge;
    *context = NULL;
    if (!b->context) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    if (GetCurrentThreadId() != b->thread) return RPC_E_WRONG_THREAD;
    if (xuiInternalContextDestroyPending(b->context) || b->context->bDestroying)
        return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    if (!n->host && (!n->widget || !xuiInternalWidgetIsValid(n->widget))) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    *context = b->context; xuiInternalOperationEnter(*context); return S_OK;
}
static HRESULT native_end(xui_context context, HRESULT result)
{ xuiInternalOperationLeave(context); return result; }
static HRESULT read_node(native_node* n, xui_accessible_node_t* out)
{
    int i, count;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (n->host) { out->iRole = XUI_ACCESSIBLE_ROLE_ROOT; out->sName = "XUI"; return S_OK; }
    if (!xuiInternalWidgetIsValid(n->widget)) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    count = xuiWidgetGetAccessibleNodeCount(n->widget);
    for (i = 0; i < count; i++) {
        if (xuiWidgetGetAccessibleNode(n->widget, i, out) != XUI_OK)
            return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
        if (out->iId == n->id) return S_OK;
    }
    return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
}
static native_node* provider(native_bridge* b, xui_widget w, uint64_t id, int host)
{
    native_node* n;
    for (n = b->nodes; n; n = n->next)
        if (n->widget == w && n->id == id && n->host == host) return n;
    /* Never reuse a runtime id, including after widget address reuse. */
    if (next_serial == LONG_MAX) return NULL;
    n = calloc(1, sizeof(*n)); if (!n) return NULL;
    n->simple.lpVtbl = &simple_vtbl; n->fragment.lpVtbl = &fragment_vtbl;
    n->root.lpVtbl = &root_vtbl; n->value.lpVtbl = &value_vtbl;
    n->invoke.lpVtbl = &invoke_vtbl; n->toggle.lpVtbl = &toggle_vtbl;
    n->refs = 1; n->bridge = b; InterlockedIncrement(&b->refs);
    n->widget = w; n->id = id; n->host = host;
    n->serial = InterlockedIncrement(&next_serial);
    n->next = b->nodes; b->nodes = n; return n;
}
static native_node* widget_root(native_bridge* b, xui_widget w)
{
    xui_accessible_node_t a = {0}; a.iSize = sizeof(a);
    if (!w || xuiWidgetGetAccessibleNode(w, 0, &a) != XUI_OK) return NULL;
    return provider(b, w, a.iId, 0);
}
static HRESULT fragment_out(native_node* n, IRawElementProviderFragment** out)
{ *out = n ? &n->fragment : NULL; if (n) native_addref(n); return S_OK; }
static BSTR wide(const char* s)
{
    int chars; BSTR result;
    if (!s) s = "";
    chars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (!chars) return NULL;
    result = SysAllocStringLen(NULL, (UINT)(chars - 1));
    if (result) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, result, chars);
    return result;
}
static int control_type(int role)
{
    switch (role) {
    case XUI_ACCESSIBLE_ROLE_DOCUMENT: return UIA_DocumentControlTypeId;
    case XUI_ACCESSIBLE_ROLE_TEXTBOX: return UIA_EditControlTypeId;
    case XUI_ACCESSIBLE_ROLE_LINK: return UIA_HyperlinkControlTypeId;
    case XUI_ACCESSIBLE_ROLE_IMAGE: return UIA_ImageControlTypeId;
    case XUI_ACCESSIBLE_ROLE_TABLE: return UIA_TableControlTypeId;
    case XUI_ACCESSIBLE_ROLE_CELL: return UIA_DataItemControlTypeId;
    case XUI_ACCESSIBLE_ROLE_CHECKBOX: return UIA_CheckBoxControlTypeId;
    case XUI_ACCESSIBLE_ROLE_LIST: return UIA_ListControlTypeId;
    case XUI_ACCESSIBLE_ROLE_LIST_ITEM: return UIA_ListItemControlTypeId;
    case XUI_ACCESSIBLE_ROLE_TEXT: case XUI_ACCESSIBLE_ROLE_HEADING:
    case XUI_ACCESSIBLE_ROLE_PARAGRAPH: case XUI_ACCESSIBLE_ROLE_CODE_BLOCK: return UIA_TextControlTypeId;
    case XUI_ACCESSIBLE_ROLE_BUTTON: return UIA_ButtonControlTypeId;
    default: return UIA_GroupControlTypeId;
    }
}
static HRESULT STDMETHODCALLTYPE options(IRawElementProviderSimple* p, enum ProviderOptions* out)
{ (void)p; if (!out) return E_POINTER;
    *out = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading; return S_OK; }
static HRESULT STDMETHODCALLTYPE patterns(IRawElementProviderSimple* p, PATTERNID id, IUnknown** out)
{
    native_node* n = NODE(IRawElementProviderSimple, simple, p);
    xui_context context; xui_accessible_node_t a; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a);
    if (SUCCEEDED(hr)) {
        if (id == UIA_ValuePatternId && a.iRole == XUI_ACCESSIBLE_ROLE_TEXTBOX && a.sValue && !(a.iState & XUI_ACCESSIBLE_STATE_PROTECTED))
            hr = native_query(n, &IID_IValueProvider, (void**)out);
        else if (id == UIA_InvokePatternId && (a.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)))
            hr = native_query(n, &IID_IInvokeProvider, (void**)out);
        else if (id == UIA_TogglePatternId && a.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX)
            hr = native_query(n, &IID_IToggleProvider, (void**)out);
    }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE property(IRawElementProviderSimple* p, PROPERTYID id, VARIANT* out)
{
    native_node* n = NODE(IRawElementProviderSimple, simple, p);
    xui_context context; xui_accessible_node_t a; HRESULT hr; const char* text = NULL;
    if (!out) return E_POINTER;
    VariantInit(out);
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a);
    if (SUCCEEDED(hr)) switch (id) {
    case UIA_ControlTypePropertyId: out->vt = VT_I4; out->lVal = n->host ? UIA_PaneControlTypeId : control_type(a.iRole); break;
    case UIA_NamePropertyId:
        text = a.sName && *a.sName ? a.sName :
            control_type(a.iRole) == UIA_TextControlTypeId ? a.sValue : ""; break;
    case UIA_HelpTextPropertyId: text = a.sDescription; break;
    case UIA_FrameworkIdPropertyId: text = "XUI"; break;
    case UIA_IsControlElementPropertyId: case UIA_IsContentElementPropertyId:
        out->vt = VT_BOOL; out->boolVal = VARIANT_TRUE; break;
    case UIA_IsEnabledPropertyId: out->vt = VT_BOOL; out->boolVal = (a.iState & XUI_ACCESSIBLE_STATE_DISABLED) ? VARIANT_FALSE : VARIANT_TRUE; break;
    case UIA_IsOffscreenPropertyId: out->vt = VT_BOOL; out->boolVal = (!IsWindowVisible(n->bridge->window) || (a.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN)) ? VARIANT_TRUE : VARIANT_FALSE; break;
    case UIA_IsPasswordPropertyId: out->vt = VT_BOOL; out->boolVal = (a.iState & XUI_ACCESSIBLE_STATE_PROTECTED) ? VARIANT_TRUE : VARIANT_FALSE; break;
    case UIA_HasKeyboardFocusPropertyId: case UIA_IsKeyboardFocusablePropertyId: {
        native_node* root = n->widget ? widget_root(n->bridge, n->widget) : NULL;
        uint32_t flag = id == UIA_HasKeyboardFocusPropertyId ? XUI_ACCESSIBLE_STATE_FOCUSED : XUI_ACCESSIBLE_STATE_FOCUSABLE;
        out->vt = VT_BOOL; out->boolVal = root == n && (a.iState & flag) ? VARIANT_TRUE : VARIANT_FALSE; break;
    }
    default: break;
    }
    if (SUCCEEDED(hr) && text) { out->vt = VT_BSTR; out->bstrVal = wide(text); if (!out->bstrVal) hr = E_OUTOFMEMORY; }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE host_provider(IRawElementProviderSimple* p, IRawElementProviderSimple** out)
{
    native_node* n = NODE(IRawElementProviderSimple, simple, p);
    xui_context context; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = n->host ? uia_host(n->bridge->window, out) : S_OK;
    return native_end(context, hr);
}
static native_node* parent_node(native_node* n, const xui_accessible_node_t* a)
{
    native_node* root = widget_root(n->bridge, n->widget);
    if (root == n) {
        xui_widget parent = xuiWidgetGetParent(n->widget);
        return parent ? widget_root(n->bridge, parent) : n->bridge->host;
    }
    return a->iParentId ? provider(n->bridge, n->widget, a->iParentId, 0) : root;
}
static HRESULT STDMETHODCALLTYPE navigate(IRawElementProviderFragment* p, enum NavigateDirection direction, IRawElementProviderFragment** out)
{
    native_node* n = NODE(IRawElementProviderFragment, fragment, p), *result = NULL;
    xui_context context; xui_accessible_node_t a, child; HRESULT hr; int i, count;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a);
    if (SUCCEEDED(hr) && n->host) {
        if (direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild)
            result = widget_root(n->bridge, xuiGetRootWidget(context));
    } else if (SUCCEEDED(hr) && direction == NavigateDirection_Parent) result = parent_node(n, &a);
    else if (SUCCEEDED(hr)) {
        uint64_t previous = 0, candidate = 0; int found = 0;
        int children = direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild;
        count = xuiWidgetGetAccessibleNodeCount(n->widget);
        for (i = 1; i < count; i++) {
            child.iSize = sizeof(child);
            if (xuiWidgetGetAccessibleNode(n->widget, i, &child) != XUI_OK) { hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE; break; }
            if (children && child.iParentId == n->id) {
                candidate = child.iId; if (direction == NavigateDirection_FirstChild) break;
            } else if (!children && child.iParentId == a.iParentId) {
                if (child.iId == n->id) {
                    found = 1;
                    if (direction == NavigateDirection_PreviousSibling) { candidate = previous; break; }
                } else if (found && direction == NavigateDirection_NextSibling) { candidate = child.iId; break; }
                previous = child.iId;
            }
        }
        if (candidate) result = provider(n->bridge, n->widget, candidate, 0);
        /* Widget children follow the semantic children of the widget root. */
        if (!candidate && children && widget_root(n->bridge, n->widget) == n)
            result = widget_root(n->bridge, direction == NavigateDirection_FirstChild ?
                xuiWidgetGetFirstChild(n->widget) : xuiWidgetGetLastChild(n->widget));
        if (!children && widget_root(n->bridge, n->widget) == n)
            result = widget_root(n->bridge, direction == NavigateDirection_NextSibling ?
                xuiWidgetGetNextSibling(n->widget) : xuiWidgetGetPrevSibling(n->widget));
    }
    if (SUCCEEDED(hr)) hr = fragment_out(result, out);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE runtime_id(IRawElementProviderFragment* p, SAFEARRAY** out)
{
    native_node* n = NODE(IRawElementProviderFragment, fragment, p);
    LONG values[2] = {UiaAppendRuntimeId, n->serial}; LONG index; xui_context context; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    if (!n->host) {
        *out = SafeArrayCreateVector(VT_I4, 0, 2); if (!*out) hr = E_OUTOFMEMORY;
        else for (index = 0; index < 2; index++) SafeArrayPutElement(*out, &index, &values[index]);
    }
    return native_end(context, hr);
}
static HRESULT node_bounds(native_node* n, struct UiaRect* out)
{
    xui_accessible_node_t a; HRESULT hr; POINT origin = {0};
    if (!ClientToScreen(n->bridge->window, &origin)) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    hr = read_node(n, &a); if (FAILED(hr)) return hr;
    if (n->host) { RECT rect; GetClientRect(n->bridge->window, &rect); *out = (struct UiaRect){origin.x, origin.y, rect.right, rect.bottom}; }
    else {
        xui_size_t viewport = xuiGetViewportSize(n->bridge->context);
        xui_rect_t rect = n->bridge->window_rect;
        double sx = viewport.iW > 0 ? rect.fW / viewport.iW : 1;
        double sy = viewport.iH > 0 ? rect.fH / viewport.iH : 1;
        *out = (struct UiaRect){origin.x + rect.fX + a.tBounds.fX * sx,
            origin.y + rect.fY + a.tBounds.fY * sy, a.tBounds.fW * sx, a.tBounds.fH * sy};
    }
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE bounds(IRawElementProviderFragment* p, struct UiaRect* out)
{
    native_node* n = NODE(IRawElementProviderFragment, fragment, p); xui_context context; HRESULT hr;
    if (!out) return E_POINTER;
    memset(out, 0, sizeof(*out));
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    return native_end(context, node_bounds(n, out));
}
static HRESULT STDMETHODCALLTYPE embedded(IRawElementProviderFragment* p, SAFEARRAY** out)
{ (void)p; if (!out) return E_POINTER;
    *out = NULL; return S_OK; }
static HRESULT action_result(int result)
{
    if (result == XUI_OK) return S_OK;
    if (result == XUI_ERROR_OUT_OF_MEMORY) return E_OUTOFMEMORY;
    if (result == XUI_ERROR_UNSUPPORTED) return (HRESULT)UIA_E_NOTSUPPORTED;
        if (result == XUI_ERROR_NOT_FOUND) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    return E_INVALIDARG;
}
static HRESULT act(native_node* n, int action, const void* data)
{
    xui_accessible_node_t a; HRESULT hr = read_node(n, &a);
    if (FAILED(hr)) return hr;
    if (a.iState & XUI_ACCESSIBLE_STATE_DISABLED) return (HRESULT)UIA_E_ELEMENTNOTENABLED;
    return action_result(xuiWidgetPerformAccessibleAction(n->widget, n->id, action, data));
}
static HRESULT STDMETHODCALLTYPE set_focus(IRawElementProviderFragment* p)
{
    native_node* n = NODE(IRawElementProviderFragment, fragment, p); xui_context context; HRESULT hr;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    if (n->host) n = widget_root(n->bridge, xuiGetFocusWidget(context) ? xuiGetFocusWidget(context) : xuiGetRootWidget(context));
    hr = n ? act(n, XUI_ACCESSIBLE_ACTION_FOCUS, NULL) : (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE fragment_root(IRawElementProviderFragment* p, IRawElementProviderFragmentRoot** out)
{
    native_node* n = NODE(IRawElementProviderFragment, fragment, p); xui_context context; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    *out = &n->bridge->host->root; native_addref(n->bridge->host); return native_end(context, S_OK);
}
static HRESULT STDMETHODCALLTYPE get_focus(IRawElementProviderFragmentRoot* p, IRawElementProviderFragment** out)
{
    native_node* n = NODE(IRawElementProviderFragmentRoot, root, p); xui_context context; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = fragment_out(widget_root(n->bridge, xuiGetFocusWidget(context)), out);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE from_point(IRawElementProviderFragmentRoot* p, double x, double y, IRawElementProviderFragment** out)
{
    native_node* n = NODE(IRawElementProviderFragmentRoot, root, p), *found = n;
    xui_context context; HRESULT hr; xui_widget w; struct UiaRect rect;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    if (FAILED(node_bounds(n, &rect)) || x < rect.left || y < rect.top || x >= rect.left + rect.width || y >= rect.top + rect.height)
        return native_end(context, S_OK);
    /* Iterative widget walk: deeply nested documents never consume C stack. */
    w = xuiGetRootWidget(context);
    while (w && !xuiInternalContextDestroyPending(context)) {
        int i, count = xuiWidgetGetAccessibleNodeCount(w);
        if (xuiWidgetGetEffectiveVisible(w)) for (i = 0; i < count; i++) {
            xui_accessible_node_t a = {0}; native_node* candidate; a.iSize = sizeof(a);
            if (xuiWidgetGetAccessibleNode(w, i, &a) != XUI_OK) break;
            candidate = provider(n->bridge, w, a.iId, 0); if (!candidate) { hr = E_OUTOFMEMORY; break; }
            if (!(a.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) && SUCCEEDED(node_bounds(candidate, &rect)) &&
                x >= rect.left && y >= rect.top && x < rect.left + rect.width && y < rect.top + rect.height) found = candidate;
        }
        if (xuiWidgetGetFirstChild(w)) w = xuiWidgetGetFirstChild(w);
        else { while (w && !xuiWidgetGetNextSibling(w)) w = xuiWidgetGetParent(w); if (w) w = xuiWidgetGetNextSibling(w); }
    }
    if (SUCCEEDED(hr)) hr = fragment_out(found, out);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE get_value(IValueProvider* p, BSTR* out)
{
    native_node* n = NODE(IValueProvider, value, p); xui_context context; xui_accessible_node_t a; HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a);
    if (SUCCEEDED(hr) && (a.iState & XUI_ACCESSIBLE_STATE_PROTECTED)) hr = E_ACCESSDENIED;
    if (SUCCEEDED(hr)) { *out = wide(a.sValue); if (!*out) hr = E_OUTOFMEMORY; }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE value_readonly(IValueProvider* p, BOOL* out)
{
    native_node* n = NODE(IValueProvider, value, p); xui_context context; xui_accessible_node_t a; HRESULT hr;
    if (!out) return E_POINTER;
    *out = TRUE;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a); if (SUCCEEDED(hr)) *out = !(a.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE set_value(IValueProvider* p, LPCWSTR value)
{
    native_node* n = NODE(IValueProvider, value, p); xui_context context; HRESULT hr; char* text; int bytes;
    xui_accessible_value_t data = {0}; if (!value) return E_INVALIDARG;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, NULL, 0, NULL, NULL);
    if (!bytes) return native_end(context, E_INVALIDARG);
    text = malloc((size_t)bytes); if (!text) return native_end(context, E_OUTOFMEMORY);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, text, bytes, NULL, NULL);
    data.iSize = sizeof(data); data.iType = XUI_ACCESSIBLE_VALUE_TEXT; data.sText = text;
    hr = act(n, XUI_ACCESSIBLE_ACTION_SET_VALUE, &data); free(text); return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE invoke_action(IInvokeProvider* p)
{
    native_node* n = NODE(IInvokeProvider, invoke, p); xui_context context; HRESULT hr = native_begin(n, &context);
    if (FAILED(hr)) return hr;
    return native_end(context, act(n, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL));
}
static HRESULT STDMETHODCALLTYPE toggle_action(IToggleProvider* p)
{
    native_node* n = NODE(IToggleProvider, toggle, p); xui_context context; HRESULT hr = native_begin(n, &context);
    if (FAILED(hr)) return hr;
    return native_end(context, act(n, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL));
}
static HRESULT STDMETHODCALLTYPE toggle_state(IToggleProvider* p, enum ToggleState* out)
{
    native_node* n = NODE(IToggleProvider, toggle, p); xui_context context; xui_accessible_node_t a; HRESULT hr;
    if (!out) return E_POINTER;
    *out = ToggleState_Off;
    hr = native_begin(n, &context); if (FAILED(hr)) return hr;
    hr = read_node(n, &a); if (SUCCEEDED(hr)) *out = a.iState & XUI_ACCESSIBLE_STATE_CHECKED ? ToggleState_On : ToggleState_Off;
    return native_end(context, hr);
}
static IRawElementProviderSimpleVtbl simple_vtbl = {simple_query, simple_addref, simple_release, options, patterns, property, host_provider};
static IRawElementProviderFragmentVtbl fragment_vtbl = {fragment_query, fragment_addref, fragment_release, navigate, runtime_id, bounds, embedded, set_focus, fragment_root};
static IRawElementProviderFragmentRootVtbl root_vtbl = {root_query, root_addref, root_release, from_point, get_focus};
static IValueProviderVtbl value_vtbl = {value_query, value_addref, value_release, set_value, get_value, value_readonly};
static IInvokeProviderVtbl invoke_vtbl = {invoke_query, invoke_addref, invoke_release, invoke_action};
static IToggleProviderVtbl toggle_vtbl = {toggle_query, toggle_addref, toggle_release, toggle_action, toggle_state};

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data)
{
    native_bridge* b = (native_bridge*)data; (void)id;
    if (message == WM_GETOBJECT && (LONG)lp == UiaRootObjectId && b->context)
        return uia_return(window, wp, lp, &b->host->simple);
    if (message == WM_NCDESTROY && b->context) xuiInternalNativeAccessibilityDetach(b->context);
    return subclass_default(window, message, wp, lp);
}
XUI_API int xuiInternalNativeAccessibilityBind(xui_context context, void* native, xui_rect_t rect)
{
    native_bridge* b; HRESULT hr; HWND window = (HWND)native;
    if (!xuiInternalContextIsValid(context) || !window || !IsWindow(window) ||
        GetWindowThreadProcessId(window, NULL) != GetCurrentThreadId()) return XUI_ERROR_INVALID_ARGUMENT;
    if (owner_thread && owner_thread != GetCurrentThreadId()) return XUI_ERROR_UNSUPPORTED;
    for (b = bridges; b; b = b->next) if (b->context == context && b->window == window) { b->window_rect = rect; return XUI_OK; }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return XUI_ERROR_UNSUPPORTED;
    if (!uia_return) {
        HMODULE uia = LoadLibraryW(L"UIAutomationCore.dll"), common = LoadLibraryW(L"comctl32.dll");
        if (!uia || !common) { CoUninitialize(); return XUI_ERROR_UNSUPPORTED; }
#define LOAD(module, target, name) do { FARPROC address = GetProcAddress(module, name); \
        _Static_assert(sizeof(address) == sizeof(target), "Windows function pointer size"); \
        memcpy(&target, &address, sizeof(target)); } while (0)
        LOAD(uia, uia_return, "UiaReturnRawElementProvider"); LOAD(uia, uia_host, "UiaHostProviderFromHwnd");
        LOAD(uia, uia_event, "UiaRaiseAutomationEvent"); LOAD(uia, uia_structure, "UiaRaiseStructureChangedEvent");
        LOAD(uia, uia_disconnect, "UiaDisconnectProvider"); LOAD(uia, uia_listening, "UiaClientsAreListening");
        LOAD(common, subclass_set, "SetWindowSubclass"); LOAD(common, subclass_remove, "RemoveWindowSubclass");
        LOAD(common, subclass_default, "DefSubclassProc");
#undef LOAD
    }
    if (!uia_return || !uia_host || !uia_event || !uia_structure || !uia_disconnect ||
        !uia_listening || !subclass_set || !subclass_remove || !subclass_default) { CoUninitialize(); return XUI_ERROR_UNSUPPORTED; }
    /* A single XGE HWND presents one active context. Switching invalidates the
     * old providers before installing the successor, without touching callbacks. */
    for (b = bridges; b;) { native_bridge* next = b->next;
        if (b->window == window || b->context == context) xuiInternalNativeAccessibilityDetach(b->context);
        b = next;
    }
    b = calloc(1, sizeof(*b)); if (!b) { CoUninitialize(); return XUI_ERROR_OUT_OF_MEMORY; }
    b->refs = 1; b->thread = GetCurrentThreadId(); b->window = window; b->context = context; b->window_rect = rect;
    b->host = provider(b, NULL, 0, 1);
    if (!b->host || !subclass_set(window, window_proc, (UINT_PTR)b, (DWORD_PTR)b)) {
        if (b->host) native_release(b->host);
        bridge_release(b); CoUninitialize(); return XUI_ERROR_OUT_OF_MEMORY;
    }
    owner_thread = b->thread; b->next = bridges; bridges = b; return XUI_OK;
}
XUI_API void xuiInternalNativeAccessibilityDetach(xui_context context)
{
    native_bridge** link = &bridges;
    while (*link) {
        native_bridge* b = *link; native_node* n;
        if (b->context != context) { link = &b->next; continue; }
        *link = b->next; subclass_remove(b->window, window_proc, (UINT_PTR)b);
        b->context = NULL; uia_return(b->window, 0, 0, NULL);
        n = b->nodes; b->nodes = b->host = NULL;
        while (n) { native_node* next = n->next; n->widget = NULL; uia_disconnect(&n->simple); native_release(n); n = next; }
        CoUninitialize(); bridge_release(b); break;
    }
}
XUI_API void xuiInternalNativeAccessibilityResetWidget(xui_widget widget)
{
    native_bridge* b;
    for (b = bridges; b; b = b->next) {
        native_node** link = &b->nodes;
        while (*link) { native_node* n = *link;
            if (!n->host && n->widget == widget) { *link = n->next; n->widget = NULL;
                uia_disconnect(&n->simple); native_release(n); }
            else link = &n->next;
        }
    }
}
XUI_API void xuiInternalNativeAccessibilityNotify(xui_context context, xui_widget widget, const xui_accessibility_event_t* event)
{
    native_bridge* b; native_node* n;
    if (!uia_listening || !uia_listening()) return;
    for (b = bridges; b; b = b->next) if (b->context == context) {
        for (n = b->nodes; n; n = n->next) if (n->widget == widget && (!event->iNodeId || event->iNodeId == n->id)) {
            if (event->iType == XUI_ACCESSIBLE_EVENT_TREE_CHANGED) uia_structure(&n->simple, StructureChangeType_ChildrenInvalidated, NULL, 0);
            else if (event->iType == XUI_ACCESSIBLE_EVENT_FOCUS_CHANGED) uia_event(&n->simple, UIA_AutomationFocusChangedEventId);
            else if (event->iType == XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED) uia_event(&n->simple, UIA_LayoutInvalidatedEventId);
        }
    }
}
#else
XUI_API int xuiInternalNativeAccessibilityBind(xui_context c, void* w, xui_rect_t r)
{ (void)c; (void)w; (void)r; return XUI_ERROR_UNSUPPORTED; }
XUI_API void xuiInternalNativeAccessibilityDetach(xui_context c) { (void)c; }
XUI_API void xuiInternalNativeAccessibilityResetWidget(xui_widget w) { (void)w; }
XUI_API void xuiInternalNativeAccessibilityNotify(xui_context c, xui_widget w, const xui_accessibility_event_t* e)
{ (void)c; (void)w; (void)e; }
#endif
#endif
