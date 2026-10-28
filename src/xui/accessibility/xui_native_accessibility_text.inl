/* UI Automation TextPattern adapter over XUI's platform-neutral UTF-8 text
 * range protocol. UIA-facing offsets and lengths are UTF-16; the retained
 * XUI range remains the only source of text, movement, and geometry. */
typedef struct native_text_range {
    ITextRangeProvider iface;
    LONG refs;
    native_node* node;
    xui_internal_accessible_text_range* range;
} native_text_range;

static ITextRangeProviderVtbl text_range_vtbl;
#define TEXT_RANGE(p) ((native_text_range*)((char*)(p) - offsetof(native_text_range, iface)))

static HRESULT text_status(int result)
{
    if (result == XUI_OK) return S_OK;
    if (result == XUI_ERROR_OUT_OF_MEMORY) return E_OUTOFMEMORY;
    if (result == XUI_ERROR_UNSUPPORTED) return (HRESULT)UIA_E_NOTSUPPORTED;
    if (result == XUI_ERROR_INVALID_ARGUMENT) return E_INVALIDARG;
    return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
}
static native_text_range* text_range_cast(ITextRangeProvider* value)
{
    if (!value || value->lpVtbl != &text_range_vtbl) return NULL;
    return TEXT_RANGE(value);
}
static HRESULT text_range_new(native_node* node,
    xui_internal_accessible_text_range* range, ITextRangeProvider** out)
{
    native_text_range* value;
    if (!out) {
        if (range) range->ops->release(range);
        return E_POINTER;
    }
    *out = NULL;
    if (!range) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    value = calloc(1, sizeof(*value));
    if (!value) {
        range->ops->release(range);
        return E_OUTOFMEMORY;
    }
    value->iface.lpVtbl = &text_range_vtbl;
    value->refs = 1;
    value->node = node;
    native_addref(node);
    value->range = range;
    *out = &value->iface;
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE text_range_query(ITextRangeProvider* p,
    REFIID iid, void** out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (!IsEqualIID(iid, &IID_IUnknown) &&
        !IsEqualIID(iid, &IID_ITextRangeProvider)) return E_NOINTERFACE;
    *out = p;
    InterlockedIncrement(&TEXT_RANGE(p)->refs);
    return S_OK;
}
static ULONG STDMETHODCALLTYPE text_range_addref(ITextRangeProvider* p)
{ return (ULONG)InterlockedIncrement(&TEXT_RANGE(p)->refs); }
static ULONG STDMETHODCALLTYPE text_range_release(ITextRangeProvider* p)
{
    native_text_range* value = TEXT_RANGE(p);
    ULONG refs = (ULONG)InterlockedDecrement(&value->refs);
    if (!refs) {
        value->range->ops->release(value->range);
        native_release(value->node);
        free(value);
    }
    return refs;
}
static HRESULT text_range_begin(native_text_range* value, xui_context* context)
{ return native_begin(value->node, context); }
static int text_endpoint(enum TextPatternRangeEndpoint endpoint)
{
    return endpoint == TextPatternRangeEndpoint_Start ? XUI_INTERNAL_TEXT_START :
        endpoint == TextPatternRangeEndpoint_End ? XUI_INTERNAL_TEXT_END : -1;
}
static int text_unit(enum TextUnit unit)
{
    switch (unit) {
    case TextUnit_Character: return XUI_INTERNAL_TEXT_GRAPHEME;
    case TextUnit_Format: return XUI_INTERNAL_TEXT_FORMAT;
    case TextUnit_Word: return XUI_INTERNAL_TEXT_WORD;
    case TextUnit_Line: return XUI_INTERNAL_TEXT_LINE;
    case TextUnit_Paragraph: return XUI_INTERNAL_TEXT_PARAGRAPH;
    /* XUI has no page model. UIA requires the next larger supported unit. */
    case TextUnit_Page: case TextUnit_Document: return XUI_INTERNAL_TEXT_ALL;
    default: return -1;
    }
}
static HRESULT text_create(native_node* node, int kind, uint64_t child,
    double x, double y, ITextRangeProvider** out)
{
    xui_internal_accessible_text_request request = {0};
    xui_internal_accessible_text_range* range = NULL;
    int result;
    request.kind = kind;
    request.child = child;
    request.x = x;
    request.y = y;
    result = xuiInternalAccessibilityCreateTextRange(node->widget, node->id,
        &request, &range);
    if (result != XUI_OK) return text_status(result);
    return text_range_new(node, range, out);
}
static HRESULT text_unknown_array(IUnknown* value, SAFEARRAY** out)
{
    SAFEARRAY* array;
    LONG index = 0;
    HRESULT hr = S_OK;
    if (!out) return E_POINTER;
    *out = NULL;
    array = SafeArrayCreateVector(VT_UNKNOWN, 0, value ? 1 : 0);
    if (!array) return E_OUTOFMEMORY;
    if (value) hr = SafeArrayPutElement(array, &index, value);
    if (FAILED(hr)) SafeArrayDestroy(array);
    else *out = array;
    return hr;
}
static HRESULT text_range_utf8(native_text_range* value, char** out,
    uint64_t* bytes)
{
    int result;
    if (!out || !bytes) return E_POINTER;
    *out = NULL;
    *bytes = 0;
    result = value->range->ops->getText(value->range, NULL, 0, bytes);
    if (result != XUI_OK) return text_status(result);
    if (*bytes > SIZE_MAX - 1) return E_OUTOFMEMORY;
    *out = malloc((size_t)*bytes + 1);
    if (!*out) return E_OUTOFMEMORY;
    result = value->range->ops->getText(value->range, *out, *bytes + 1, bytes);
    if (result != XUI_OK) {
        free(*out);
        *out = NULL;
        return text_status(result);
    }
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE text_range_clone(ITextRangeProvider* p,
    ITextRangeProvider** out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_internal_accessible_text_range* clone = NULL;
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->clone(value->range, &clone));
    if (SUCCEEDED(hr)) hr = text_range_new(value->node, clone, out);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_compare(ITextRangeProvider* p,
    ITextRangeProvider* other, WINBOOL* out)
{
    native_text_range* value = TEXT_RANGE(p);
    native_text_range* peer = text_range_cast(other);
    xui_context context;
    HRESULT hr;
    int equal = 0;
    if (!out) return E_POINTER;
    *out = FALSE;
    if (!peer) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->compare(value->range, peer->range, &equal));
    if (SUCCEEDED(hr)) *out = equal ? TRUE : FALSE;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_compare_endpoints(ITextRangeProvider* p,
    enum TextPatternRangeEndpoint endpoint, ITextRangeProvider* other,
    enum TextPatternRangeEndpoint other_endpoint, int* out)
{
    native_text_range* value = TEXT_RANGE(p);
    native_text_range* peer = text_range_cast(other);
    xui_context context;
    HRESULT hr;
    int first = text_endpoint(endpoint), second = text_endpoint(other_endpoint);
    if (!out) return E_POINTER;
    *out = 0;
    if (!peer || first < 0 || second < 0) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->compareEndpoints(value->range, first,
        peer->range, second, out));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_expand(ITextRangeProvider* p,
    enum TextUnit unit)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr;
    int mapped = text_unit(unit);
    if (mapped < 0) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->expand(value->range, mapped));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_find_attribute(ITextRangeProvider* p,
    TEXTATTRIBUTEID attribute, VARIANT value, WINBOOL backward,
    ITextRangeProvider** out)
{
    native_text_range* range = TEXT_RANGE(p);
    xui_context context;
    xui_accessible_node_t node;
    xui_internal_accessible_text_range* clone = NULL;
    HRESULT hr;
    (void)backward;
    if (!out) return E_POINTER;
    *out = NULL;
    if (attribute != UIA_IsReadOnlyAttributeId || value.vt != VT_BOOL)
        return S_OK;
    hr = text_range_begin(range, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(range->node, &node);
    if (SUCCEEDED(hr) && value.boolVal ==
        ((node.iState & XUI_ACCESSIBLE_STATE_READONLY) ? VARIANT_TRUE : VARIANT_FALSE)) {
        hr = text_status(range->range->ops->clone(range->range, &clone));
        if (SUCCEEDED(hr)) hr = text_range_new(range->node, clone, out);
    }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_find_text(ITextRangeProvider* p,
    BSTR text, WINBOOL backward, WINBOOL ignore_case, ITextRangeProvider** out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_internal_accessible_text_range* found = NULL;
    xui_context context;
    HRESULT hr;
    char* utf8;
    UINT chars;
    int bytes, result;
    if (!out) return E_POINTER;
    *out = NULL;
    if (!text || !(chars = SysStringLen(text))) return E_INVALIDARG;
    bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text,
        (int)chars, NULL, 0, NULL, NULL);
    if (bytes <= 0) return E_INVALIDARG;
    utf8 = malloc((size_t)bytes);
    if (!utf8) return E_OUTOFMEMORY;
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text,
        (int)chars, utf8, bytes, NULL, NULL)) {
        free(utf8);
        return E_INVALIDARG;
    }
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) {
        free(utf8);
        return hr;
    }
    result = value->range->ops->findText(value->range, utf8, (uint64_t)bytes,
        backward != FALSE, ignore_case != FALSE, &found);
    free(utf8);
    hr = text_status(result);
    if (SUCCEEDED(hr) && found) hr = text_range_new(value->node, found, out);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_attribute(ITextRangeProvider* p,
    TEXTATTRIBUTEID attribute, VARIANT* out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    xui_accessible_node_t node;
    HRESULT hr;
    IUnknown* unsupported = NULL;
    if (!out) return E_POINTER;
    VariantInit(out);
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    if (attribute == UIA_IsReadOnlyAttributeId) {
        hr = read_node(value->node, &node);
        if (SUCCEEDED(hr)) {
            out->vt = VT_BOOL;
            out->boolVal = (node.iState & XUI_ACCESSIBLE_STATE_READONLY) ?
                VARIANT_TRUE : VARIANT_FALSE;
        }
    } else {
        hr = uia_not_supported(&unsupported);
        if (SUCCEEDED(hr)) {
            out->vt = VT_UNKNOWN;
            out->punkVal = unsupported;
        }
    }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_rectangles(ITextRangeProvider* p,
    SAFEARRAY** out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    xui_rect_t* rects = NULL;
    SAFEARRAY* array = NULL;
    xui_size_t viewport;
    POINT origin = {0};
    uint64_t count = 0, i;
    double sx, sy;
    HRESULT hr;
    int result;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    result = value->range->ops->getRects(value->range, NULL, 0, &count);
    if (result == XUI_OK && count > (uint64_t)LONG_MAX / 4)
        result = XUI_ERROR_OUT_OF_MEMORY;
    if (result == XUI_OK && count) {
        rects = malloc((size_t)count * sizeof(*rects));
        if (!rects) result = XUI_ERROR_OUT_OF_MEMORY;
        else result = value->range->ops->getRects(value->range, rects, count, &count);
    }
    hr = text_status(result);
    if (SUCCEEDED(hr)) {
        array = SafeArrayCreateVector(VT_R8, 0, (ULONG)(count * 4));
        if (!array) hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr) && !ClientToScreen(value->node->bridge->window, &origin))
        hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    viewport = xuiGetViewportSize(context);
    sx = viewport.iW > 0 ? value->node->bridge->window_rect.fW / viewport.iW : 1;
    sy = viewport.iH > 0 ? value->node->bridge->window_rect.fH / viewport.iH : 1;
    for (i = 0; SUCCEEDED(hr) && i < count; i++) {
        double values[4] = {
            origin.x + value->node->bridge->window_rect.fX + rects[i].fX * sx,
            origin.y + value->node->bridge->window_rect.fY + rects[i].fY * sy,
            rects[i].fW * sx, rects[i].fH * sy
        };
        int field;
        for (field = 0; field < 4; field++) {
            LONG index = (LONG)(i * 4 + (uint64_t)field);
            hr = SafeArrayPutElement(array, &index, &values[field]);
            if (FAILED(hr)) break;
        }
    }
    free(rects);
    if (FAILED(hr)) SafeArrayDestroy(array);
    else *out = array;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_enclosing(ITextRangeProvider* p,
    IRawElementProviderSimple** out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    *out = &value->node->simple;
    native_addref(value->node);
    return native_end(context, S_OK);
}
static HRESULT STDMETHODCALLTYPE text_range_get_text(ITextRangeProvider* p,
    int max_length, BSTR* out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    char* utf8 = NULL;
    uint64_t bytes = 0;
    int chars = 0, kept;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    if (max_length < -1) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_range_utf8(value, &utf8, &bytes);
    if (SUCCEEDED(hr) && bytes) {
        if (bytes > INT_MAX) hr = E_OUTOFMEMORY;
        else {
            chars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                utf8, (int)bytes, NULL, 0);
            if (!chars) hr = E_INVALIDARG;
        }
    }
    kept = max_length < 0 || max_length > chars ? chars : max_length;
    if (kept > 0) {
        WCHAR* buffer = malloc((size_t)chars * sizeof(*buffer));
        if (!buffer) hr = E_OUTOFMEMORY;
        else {
            if (SUCCEEDED(hr) && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                utf8, (int)bytes, buffer, chars) != chars) hr = E_INVALIDARG;
            if (SUCCEEDED(hr) && kept < chars && kept > 0 &&
                buffer[kept - 1] >= 0xd800 && buffer[kept - 1] <= 0xdbff) kept--;
            if (SUCCEEDED(hr)) {
                *out = SysAllocStringLen(buffer, (UINT)kept);
                if (!*out) hr = E_OUTOFMEMORY;
            }
            free(buffer);
        }
    } else if (SUCCEEDED(hr)) {
        *out = SysAllocStringLen(NULL, 0);
        if (!*out) hr = E_OUTOFMEMORY;
    }
    free(utf8);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_move(ITextRangeProvider* p,
    enum TextUnit unit, int count, int* out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr;
    int mapped = text_unit(unit);
    if (!out) return E_POINTER;
    *out = 0;
    if (mapped < 0) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->move(value->range, mapped, count, out));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_move_endpoint_unit(ITextRangeProvider* p,
    enum TextPatternRangeEndpoint endpoint, enum TextUnit unit, int count, int* out)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr;
    int edge = text_endpoint(endpoint), mapped = text_unit(unit);
    if (!out) return E_POINTER;
    *out = 0;
    if (edge < 0 || mapped < 0) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->moveEndpoint(value->range, edge,
        mapped, count, out));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_move_endpoint_range(ITextRangeProvider* p,
    enum TextPatternRangeEndpoint endpoint, ITextRangeProvider* other,
    enum TextPatternRangeEndpoint other_endpoint)
{
    native_text_range* value = TEXT_RANGE(p);
    native_text_range* peer = text_range_cast(other);
    xui_context context;
    HRESULT hr;
    int edge = text_endpoint(endpoint), peer_edge = text_endpoint(other_endpoint);
    if (!peer || edge < 0 || peer_edge < 0) return E_INVALIDARG;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->setEndpoint(value->range, edge,
        peer->range, peer_edge));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_select(ITextRangeProvider* p)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->select(value->range));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_add_selection(ITextRangeProvider* p)
{ (void)p; return (HRESULT)UIA_E_INVALIDOPERATION; }
static HRESULT STDMETHODCALLTYPE text_range_remove_selection(ITextRangeProvider* p)
{ (void)p; return (HRESULT)UIA_E_INVALIDOPERATION; }
static HRESULT STDMETHODCALLTYPE text_range_scroll(ITextRangeProvider* p,
    WINBOOL align_top)
{
    native_text_range* value = TEXT_RANGE(p);
    xui_context context;
    HRESULT hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    hr = text_status(value->range->ops->scrollIntoView(value->range,
        align_top != FALSE));
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_children(ITextRangeProvider* p,
    SAFEARRAY** out)
{
    native_text_range* value = TEXT_RANGE(p);
    native_node* owner = value->node;
    xui_context context;
    SAFEARRAY* array = NULL;
    IUnknown** children = NULL;
    int count, i, used = 0;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = text_range_begin(value, &context);
    if (FAILED(hr)) return hr;
    count = xuiWidgetGetAccessibleNodeCount(owner->widget);
    if (count > 1) {
        children = calloc((size_t)(count - 1), sizeof(*children));
        if (!children) hr = E_OUTOFMEMORY;
    }
    for (i = 1; SUCCEEDED(hr) && i < count; i++) {
        xui_accessible_node_t node = {0};
        xui_internal_accessible_text_request request = {0};
        xui_internal_accessible_text_range* child = NULL;
        int start = 0, end = 0, result;
        node.iSize = sizeof(node);
        if (xuiWidgetGetAccessibleNode(owner->widget, i, &node) != XUI_OK) {
            hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
            break;
        }
        if (node.iId == owner->id || node.iParentId != owner->id) continue;
        request.kind = XUI_INTERNAL_TEXT_CHILD;
        request.child = node.iId;
        result = xuiInternalAccessibilityCreateTextRange(owner->widget,
            owner->id, &request, &child);
        if (result == XUI_ERROR_UNSUPPORTED || result == XUI_ERROR_INVALID_ARGUMENT)
            continue;
        if (result != XUI_OK) {
            hr = text_status(result);
            break;
        }
        result = value->range->ops->compareEndpoints(value->range,
            XUI_INTERNAL_TEXT_START, child, XUI_INTERNAL_TEXT_START, &start);
        if (result == XUI_OK) result = value->range->ops->compareEndpoints(
            value->range, XUI_INTERNAL_TEXT_END, child, XUI_INTERNAL_TEXT_END, &end);
        child->ops->release(child);
        if (result != XUI_OK) {
            hr = text_status(result);
            break;
        }
        if (start <= 0 && end >= 0) {
            native_node* provider_node = provider(owner->bridge, owner->widget,
                node.iId, 0);
            if (!provider_node) {
                hr = E_OUTOFMEMORY;
                break;
            }
            children[used++] = (IUnknown*)&provider_node->simple;
        }
    }
    if (SUCCEEDED(hr)) {
        LONG index;
        array = SafeArrayCreateVector(VT_UNKNOWN, 0, (ULONG)used);
        if (!array) hr = E_OUTOFMEMORY;
        for (index = 0; SUCCEEDED(hr) && index < used; index++)
            hr = SafeArrayPutElement(array, &index, children[index]);
    }
    free(children);
    if (FAILED(hr)) SafeArrayDestroy(array);
    else *out = array;
    return native_end(context, hr);
}

static ITextRangeProviderVtbl text_range_vtbl = {
    text_range_query, text_range_addref, text_range_release,
    text_range_clone, text_range_compare, text_range_compare_endpoints,
    text_range_expand, text_range_find_attribute, text_range_find_text,
    text_range_attribute, text_range_rectangles, text_range_enclosing,
    text_range_get_text, text_range_move, text_range_move_endpoint_unit,
    text_range_move_endpoint_range, text_range_select,
    text_range_add_selection, text_range_remove_selection,
    text_range_scroll, text_range_children
};

static HRESULT text_provider_range(native_node* node, int kind,
    uint64_t child, double x, double y, ITextRangeProvider** out)
{
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = text_create(node, kind, child, x, y, out);
    return native_end(context, hr);
}
static HRESULT text_provider_array(native_node* node, int kind, SAFEARRAY** out)
{
    ITextRangeProvider* range = NULL;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = text_provider_range(node, kind, 0, 0, 0, &range);
    if (hr == (HRESULT)UIA_E_NOTSUPPORTED && kind == XUI_INTERNAL_TEXT_SELECTION)
        return text_unknown_array(NULL, out);
    if (FAILED(hr)) return hr;
    hr = text_unknown_array((IUnknown*)range, out);
    ITextRangeProvider_Release(range);
    return hr;
}
static HRESULT STDMETHODCALLTYPE text_get_selection(ITextProvider* p,
    SAFEARRAY** out)
{ return text_provider_array(NODE(ITextProvider, text, p), XUI_INTERNAL_TEXT_SELECTION, out); }
static HRESULT STDMETHODCALLTYPE text_get_visible_ranges(ITextProvider* p,
    SAFEARRAY** out)
{
    native_node* node = NODE(ITextProvider, text, p);
    xui_context context;
    xui_rect_t world, content;
    ITextRangeProvider *first = NULL, *last = NULL;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    world = xuiWidgetGetWorldRect(node->widget);
    content = xuiWidgetGetContentRect(node->widget);
    if (content.fW <= 1 || content.fH <= 1) {
        hr = text_create(node, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0, &first);
    } else {
        hr = text_create(node, XUI_INTERNAL_TEXT_POINT, 0,
            world.fX + content.fX + .5, world.fY + content.fY + .5, &first);
        if (SUCCEEDED(hr)) hr = text_create(node, XUI_INTERNAL_TEXT_POINT, 0,
            world.fX + content.fX + content.fW - .5,
            world.fY + content.fY + content.fH - .5, &last);
        if (SUCCEEDED(hr)) hr = ITextRangeProvider_MoveEndpointByRange(first,
            TextPatternRangeEndpoint_End, last, TextPatternRangeEndpoint_End);
    }
    if (last) ITextRangeProvider_Release(last);
    if (SUCCEEDED(hr)) hr = text_unknown_array((IUnknown*)first, out);
    if (first) ITextRangeProvider_Release(first);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE text_range_from_child(ITextProvider* p,
    IRawElementProviderSimple* child, ITextRangeProvider** out)
{
    native_node* node = NODE(ITextProvider, text, p);
    native_node* candidate;
    if (!out) return E_POINTER;
    *out = NULL;
    if (!child) return E_INVALIDARG;
    for (candidate = node->bridge->nodes; candidate; candidate = candidate->next)
        if (&candidate->simple == child && candidate->widget == node->widget &&
            !candidate->host)
            return text_provider_range(node, XUI_INTERNAL_TEXT_CHILD,
                candidate->id, 0, 0, out);
    return E_INVALIDARG;
}
static HRESULT STDMETHODCALLTYPE text_range_from_point(ITextProvider* p,
    struct UiaPoint point, ITextRangeProvider** out)
{
    native_node* node = NODE(ITextProvider, text, p);
    xui_size_t viewport;
    POINT origin = {0};
    double sx, sy, x, y;
    if (!out) return E_POINTER;
    *out = NULL;
    if (!isfinite(point.x) || !isfinite(point.y)) return E_INVALIDARG;
    if (!ClientToScreen(node->bridge->window, &origin))
        return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    viewport = xuiGetViewportSize(node->bridge->context);
    sx = viewport.iW > 0 ? node->bridge->window_rect.fW / viewport.iW : 1;
    sy = viewport.iH > 0 ? node->bridge->window_rect.fH / viewport.iH : 1;
    if (sx <= 0 || sy <= 0) return (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    x = (point.x - origin.x - node->bridge->window_rect.fX) / sx;
    y = (point.y - origin.y - node->bridge->window_rect.fY) / sy;
    return text_provider_range(node, XUI_INTERNAL_TEXT_POINT, 0, x, y, out);
}
static HRESULT STDMETHODCALLTYPE text_document_range(ITextProvider* p,
    ITextRangeProvider** out)
{ return text_provider_range(NODE(ITextProvider, text, p), XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0, out); }
static HRESULT STDMETHODCALLTYPE text_supported_selection(ITextProvider* p,
    enum SupportedTextSelection* out)
{
    native_node* node = NODE(ITextProvider, text, p);
    xui_context context;
    xui_accessible_node_t accessible;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = SupportedTextSelection_None;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &accessible);
    if (SUCCEEDED(hr) && (accessible.iState & XUI_ACCESSIBLE_STATE_SELECTABLE))
        *out = SupportedTextSelection_Single;
    return native_end(context, hr);
}
