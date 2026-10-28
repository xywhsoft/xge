/* Collection and window patterns shared by semantic controls. */
static HRESULT selection_append(native_node*** items, int* count, int* capacity,
    native_node* item)
{
    native_node** resized;
    int next;
    if (*count == *capacity) {
        next = *capacity ? *capacity * 2 : 4;
        if (next < *capacity || (size_t)next > SIZE_MAX / sizeof(**items))
            return E_OUTOFMEMORY;
        resized = realloc(*items, (size_t)next * sizeof(**items));
        if (!resized) return E_OUTOFMEMORY;
        *items = resized;
        *capacity = next;
    }
    (*items)[(*count)++] = item;
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE selection_get(ISelectionProvider* p,
    SAFEARRAY** out)
{
    native_node* node = NODE(ISelectionProvider, selection, p);
    native_node** selected = NULL;
    xui_context context;
    SAFEARRAY* array = NULL;
    HRESULT hr;
    int count = 0, capacity = 0, i, semantic_count;
    LONG index;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    semantic_count = xuiWidgetGetAccessibleNodeCount(node->widget);
    for (i = 1; SUCCEEDED(hr) && i < semantic_count; i++) {
        xui_accessible_node_t child = {0};
        native_node* item;
        child.iSize = sizeof(child);
        if (xuiWidgetGetAccessibleNode(node->widget, i, &child) != XUI_OK) {
            hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
            break;
        }
        if (!(child.iState & XUI_ACCESSIBLE_STATE_SELECTED)) continue;
        item = provider(node->bridge, node->widget, child.iId, 0);
        if (!item) hr = E_OUTOFMEMORY;
        else hr = selection_append(&selected, &count, &capacity, item);
    }
    /* RadioGroup and similar composite widgets expose selection through their
     * child widgets instead of virtual semantic children. */
    if (SUCCEEDED(hr) && semantic_count <= 1) {
        xui_widget child_widget;
        for (child_widget = xuiWidgetGetFirstChild(node->widget); child_widget;
            child_widget = xuiWidgetGetNextSibling(child_widget)) {
            xui_accessible_node_t child = {0};
            native_node* item;
            child.iSize = sizeof(child);
            if (xuiWidgetGetAccessibleNode(child_widget, 0, &child) != XUI_OK)
                continue;
            if (!(child.iState & XUI_ACCESSIBLE_STATE_SELECTED)) continue;
            item = provider(node->bridge, child_widget, child.iId, 0);
            if (!item) hr = E_OUTOFMEMORY;
            else hr = selection_append(&selected, &count, &capacity, item);
            if (FAILED(hr)) break;
        }
    }
    if (SUCCEEDED(hr)) {
        array = SafeArrayCreateVector(VT_UNKNOWN, 0, (ULONG)count);
        if (!array) hr = E_OUTOFMEMORY;
        for (index = 0; SUCCEEDED(hr) && index < count; index++)
            hr = SafeArrayPutElement(array, &index,
                (IUnknown*)&selected[index]->simple);
    }
    free(selected);
    if (FAILED(hr)) SafeArrayDestroy(array);
    else *out = array;
    return native_end(context, hr);
}
static HRESULT selection_property(ISelectionProvider* p, WINBOOL* out, uint32_t state)
{
    native_node* node = NODE(ISelectionProvider, selection, p);
    xui_context context;
    xui_accessible_node_t accessible;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = FALSE;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &accessible);
    if (SUCCEEDED(hr)) *out = (accessible.iState & state) ? TRUE : FALSE;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE selection_multiple(ISelectionProvider* p,
    WINBOOL* out)
{ return selection_property(p, out, XUI_ACCESSIBLE_STATE_MULTISELECTABLE); }
static HRESULT STDMETHODCALLTYPE selection_required(ISelectionProvider* p,
    WINBOOL* out)
{ return selection_property(p, out, XUI_ACCESSIBLE_STATE_SELECTION_REQUIRED); }

static HRESULT grid_metric(native_node* node, int* out, int column)
{
    xui_context context;
    xui_accessible_node_t accessible;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = 0;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &accessible);
    if (SUCCEEDED(hr)) {
        *out = column ? accessible.iColumnCount : accessible.iRowCount;
        if (*out < 0) hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE grid_row_count(IGridProvider* p, int* out)
{ return grid_metric(NODE(IGridProvider, grid, p), out, 0); }
static HRESULT STDMETHODCALLTYPE grid_column_count(IGridProvider* p, int* out)
{ return grid_metric(NODE(IGridProvider, grid, p), out, 1); }
static HRESULT STDMETHODCALLTYPE grid_get_item(IGridProvider* p, int row, int column,
    IRawElementProviderSimple** out)
{
    native_node* node = NODE(IGridProvider, grid, p);
    xui_context context;
    xui_accessible_node_t table;
    HRESULT hr;
    int count, i;
    if (!out) return E_POINTER;
    *out = NULL;
    if (row < 0 || column < 0) return E_INVALIDARG;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &table);
    if (SUCCEEDED(hr) && (row >= table.iRowCount || column >= table.iColumnCount))
        hr = E_INVALIDARG;
    count = SUCCEEDED(hr) ? xuiWidgetGetAccessibleNodeCount(node->widget) : 0;
    for (i = 0; SUCCEEDED(hr) && i < count; i++) {
        xui_accessible_node_t cell = {0};
        int row_span, column_span;
        native_node* result;
        cell.iSize = sizeof(cell);
        if (xuiWidgetGetAccessibleNode(node->widget, i, &cell) != XUI_OK) {
            hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
            break;
        }
        if (cell.iRole != XUI_ACCESSIBLE_ROLE_CELL) continue;
        row_span = cell.iRowCount > 0 ? cell.iRowCount : 1;
        column_span = cell.iColumnCount > 0 ? cell.iColumnCount : 1;
        if (cell.iRow < 0 || cell.iColumn < 0 || row < cell.iRow || column < cell.iColumn ||
            row - cell.iRow >= row_span || column - cell.iColumn >= column_span) continue;
        result = provider(node->bridge, node->widget, cell.iId, 0);
        if (!result) hr = E_OUTOFMEMORY;
        else { *out = &result->simple; native_addref(result); }
        break;
    }
    if (SUCCEEDED(hr) && !*out) hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    return native_end(context, hr);
}

static HRESULT grid_item_metric(native_node* node, int* out, int field)
{
    xui_context context;
    xui_accessible_node_t accessible;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = 0;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &accessible);
    if (SUCCEEDED(hr)) {
        if (field == 0) *out = accessible.iRow;
        else if (field == 1) *out = accessible.iColumn;
        else if (field == 2) *out = accessible.iRowCount > 0 ? accessible.iRowCount : 1;
        else *out = accessible.iColumnCount > 0 ? accessible.iColumnCount : 1;
        if (*out < 0) hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    }
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE grid_item_row(IGridItemProvider* p, int* out)
{ return grid_item_metric(NODE(IGridItemProvider, grid_item, p), out, 0); }
static HRESULT STDMETHODCALLTYPE grid_item_column(IGridItemProvider* p, int* out)
{ return grid_item_metric(NODE(IGridItemProvider, grid_item, p), out, 1); }
static HRESULT STDMETHODCALLTYPE grid_item_row_span(IGridItemProvider* p, int* out)
{ return grid_item_metric(NODE(IGridItemProvider, grid_item, p), out, 2); }
static HRESULT STDMETHODCALLTYPE grid_item_column_span(IGridItemProvider* p, int* out)
{ return grid_item_metric(NODE(IGridItemProvider, grid_item, p), out, 3); }
static HRESULT STDMETHODCALLTYPE grid_item_containing_grid(IGridItemProvider* p,
    IRawElementProviderSimple** out)
{
    native_node* node = NODE(IGridItemProvider, grid_item, p);
    xui_context context;
    xui_accessible_node_t current;
    HRESULT hr;
    int count, depth;
    if (!out) return E_POINTER;
    *out = NULL;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &current);
    count = SUCCEEDED(hr) ? xuiWidgetGetAccessibleNodeCount(node->widget) : 0;
    for (depth = 0; SUCCEEDED(hr) && current.iParentId && depth < count; depth++) {
        xui_accessible_node_t parent = {0};
        int i, found = 0;
        parent.iSize = sizeof(parent);
        for (i = 0; i < count; i++) {
            if (xuiWidgetGetAccessibleNode(node->widget, i, &parent) != XUI_OK) {
                hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
                break;
            }
            if (parent.iId == current.iParentId) { found = 1; break; }
        }
        if (FAILED(hr)) break;
        if (!found) { hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE; break; }
        if (parent.iRole == XUI_ACCESSIBLE_ROLE_TABLE) {
            native_node* result = provider(node->bridge, node->widget, parent.iId, 0);
            if (!result) hr = E_OUTOFMEMORY;
            else { *out = &result->simple; native_addref(result); }
            break;
        }
        current = parent;
    }
    if (SUCCEEDED(hr) && !*out) hr = (HRESULT)UIA_E_ELEMENTNOTAVAILABLE;
    return native_end(context, hr);
}

static HRESULT STDMETHODCALLTYPE window_visual_set(IWindowProvider* p,
    enum WindowVisualState state)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr;
    if (state != WindowVisualState_Normal && state != WindowVisualState_Maximized &&
        state != WindowVisualState_Minimized) return E_INVALIDARG;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = state == WindowVisualState_Normal ? S_OK : (HRESULT)UIA_E_INVALIDOPERATION;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE window_close(IWindowProvider* p)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = act(node, XUI_ACCESSIBLE_ACTION_CLOSE, NULL);
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE window_wait(IWindowProvider* p, int milliseconds,
    WINBOOL* out)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = FALSE;
    if (milliseconds < 0) return E_INVALIDARG;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    /* Calls execute on XUI's owning STA, so reaching the provider already
     * means all older window messages have been dispatched. */
    *out = TRUE;
    return native_end(context, S_OK);
}
static HRESULT window_false(IWindowProvider* p, WINBOOL* out)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = FALSE;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    return native_end(context, S_OK);
}
static HRESULT STDMETHODCALLTYPE window_can_maximize(IWindowProvider* p, WINBOOL* out)
{ return window_false(p, out); }
static HRESULT STDMETHODCALLTYPE window_can_minimize(IWindowProvider* p, WINBOOL* out)
{ return window_false(p, out); }
static HRESULT STDMETHODCALLTYPE window_topmost(IWindowProvider* p, WINBOOL* out)
{ return window_false(p, out); }
static HRESULT STDMETHODCALLTYPE window_modal(IWindowProvider* p, WINBOOL* out)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    xui_accessible_node_t accessible;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = FALSE;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    hr = read_node(node, &accessible);
    if (SUCCEEDED(hr))
        *out = (accessible.iState & XUI_ACCESSIBLE_STATE_MODAL) ? TRUE : FALSE;
    return native_end(context, hr);
}
static HRESULT STDMETHODCALLTYPE window_visual(IWindowProvider* p,
    enum WindowVisualState* out)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = WindowVisualState_Normal;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    return native_end(context, S_OK);
}
static HRESULT STDMETHODCALLTYPE window_interaction(IWindowProvider* p,
    enum WindowInteractionState* out)
{
    native_node* node = NODE(IWindowProvider, window, p);
    xui_context context;
    HRESULT hr;
    if (!out) return E_POINTER;
    *out = WindowInteractionState_ReadyForUserInteraction;
    hr = native_begin(node, &context);
    if (FAILED(hr)) return hr;
    return native_end(context, S_OK);
}
