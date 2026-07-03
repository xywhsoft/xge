# XUI

> XUI incubation APIs: retained widgets, layout, host bridge, event dispatch, text editing, and standard controls.

[Back to API Reference](README.en.md) | [Chinese Source](xui.md)

---

## Module Role

XUI incubation APIs: retained widgets, layout, host bridge, event dispatch, text editing, and standard controls.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## DockPanel / DockLayout

DockPanel is exposed through `xge_xui_dock_layout_t` and `xge_xui_dock_window_t`. The internal model is `docklayout -> dock_region -> dock_node(split|pane) -> dock_pane -> dockwindow tabs`; split nodes use an explicit binary tree so the C implementation can be maintained, tested, and serialized without copying DockPanelSuite's `previousPane + alignment + proportion` model.

`xge_xui_dock_window_t` composes an existing `xge_xui_window_t`. When docked, the regular window chrome is hidden and the dock pane paints tabs, title chrome, pane buttons, and the client slot. When floating, the internal `xge_xui_window_t` is restored as a special dockwindow and clamped inside the current XUI root; no OS/native child window is created.

Minimal setup:

```c
xge_xui_widget layoutWidget = xgeXuiWidgetCreate();
xge_xui_dock_layout_t layout;
xge_xui_dock_window_t doc;

xgeXuiDockLayoutInit(&layout, &xui, layoutWidget);
xgeXuiDockWindowInit(&doc, &xui);
xgeXuiDockWindowSetTitle(&doc, "Document.c");
xgeXuiDockWindowSetClosable(&doc, 0);
xgeXuiDockWindowSetDockable(&doc, 1);
xgeXuiDockWindowSetClientWidget(&doc, editorWidget);
xgeXuiDockLayoutDockWindow(&layout, &doc, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
```

Main APIs:

```c
XGE_API int xgeXuiDockLayoutInit(xge_xui_dock_layout pLayout, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiDockLayoutUnit(xge_xui_dock_layout pLayout);
XGE_API xge_xui_dock_pane xgeXuiDockLayoutDockWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, int iRegion, int iSide, float fProportion);
XGE_API int xgeXuiDockLayoutFloatWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_rect_t tRect);
XGE_API int xgeXuiDockLayoutHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutDockAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API xvalue xgeXuiDockLayoutSaveState(const xge_xui_dock_layout pLayout);
XGE_API int xgeXuiDockLayoutLoadState(xge_xui_dock_layout pLayout, xvalue pState);
XGE_API void xgeXuiDockLayoutStateFree(xvalue pState);
XGE_API int xgeXuiDockLayoutStateGetCounts(xvalue pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount);
XGE_API int xgeXuiDockWindowInit(xge_xui_dock_window pWindow, xge_xui_context pContext);
XGE_API void xgeXuiDockWindowUnit(xge_xui_dock_window pWindow);
XGE_API void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window pWindow, xge_xui_widget pClient);
XGE_API void xgeXuiDockWindowSetTitle(xge_xui_dock_window pWindow, const char* sTitle);
XGE_API void xgeXuiDockWindowSetClosable(xge_xui_dock_window pWindow, int bClosable);
XGE_API void xgeXuiDockWindowSetDockable(xge_xui_dock_window pWindow, int bDockable);
XGE_API int xgeXuiDockWindowGetState(const xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane pPane);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetWindow(const xge_xui_dock_pane pPane, int iIndex);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane pPane);
XGE_API void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane pPane, int iIndex);
```

The client widget passed to `xgeXuiDockWindowSetClientWidget` remains caller-owned. Docking reparents it into the dock pane slot; floating reparents it back into the internal window client slot; hiding removes the dockwindow from visible pane/floating lists while preserving recoverable state. Dock drag uses XUI pointer capture, overlay root, and `XGE_XUI_LAYER_DRAG_ADORNER`; `Escape` cancels drag, `Tab` does not leak through capture, and `XGE_KEY_MOD_CTRL` suppresses docking while keeping a floating preview.

`xgeXuiDockLayoutAutoHideWindow` moves a docked, dockable window into an edge strip with `XGE_XUI_DOCK_WINDOW_AUTO_HIDE`. Clicking the strip opens a temporary overlay pane without modifying the split tree; its dock button restores the window to the saved region/side, and close/outside click/Escape collapse it back to the strip. `xgeXuiDockLayoutDockAutoHideWindow` provides the same restore path programmatically. `xgeXuiDockLayoutSaveState` returns a caller-owned XValue table containing regions, split nodes, pane tab ids, floating rects, auto-hide regions, and dockwindow state. Release it with `xgeXuiDockLayoutStateFree`. `xgeXuiDockLayoutLoadState` restores that table against the dockwindows already registered in the layout, matching saved ids by widget name first and title as fallback; unknown or duplicate ids fail without changing the current layout.

XSON can declare an initial DockPanel:

```json
{
  "type": "dockLayout",
  "id": "mainDock",
  "regions": { "left": 0.22, "right": 0.2, "bottom": 0.25 },
  "dockWindows": [
    {
      "id": "document",
      "title": "Document",
      "region": "document",
      "side": "fill",
      "closable": false,
      "children": [ { "type": "label", "text": "Document client" } ]
    },
    {
      "id": "preview",
      "title": "Preview",
      "state": "floating",
      "rect": [560, 74, 260, 170]
    }
  ]
}
```

`dockWindows` supports `state:"docked"`, `state:"floating"`, `state:"hidden"`, and `state:"autoHide"`. `region` accepts `document/left/right/top/bottom`; `side` accepts `fill/left/right/top/bottom`. Declarative children attach to the dockwindow client and are released by `xgeXuiPageUnload`. XSON `autoHide` windows start collapsed in their strip and use the same overlay expansion path as interactive auto-hide windows. Application-level persistence file format and full split-tree XSON roundtrip are still tracked as later phases; runtime XValue save/load is available for applications that want to store their own configuration.

## Function Reference

### xgeXuiSizePx

Provides the `xgeXuiSizePx` operation.

**Purpose:**

Provides the `xgeXuiSizePx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
```

**Parameters:**

- `fValue`: `float fValue`.

**Return Value:**

- Returns a `xge_xui_size_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- `xgeXuiSetTheme` increments the context theme version and marks the root layout/paint dirty. The version is intended for style/token cache invalidation.
- `xgeXuiTokenSetColor`, `xgeXuiTokenSetSpacing`, and `xgeXuiTokenSetFont` register context-level fallback tokens. Successful registration increments the theme version and marks the root style/layout/paint dirty. Font tokens store borrowed font pointers.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSizeDip

Provides the `xgeXuiSizeDip` operation.

**Purpose:**

Provides the `xgeXuiSizeDip` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_size_t xgeXuiSizeDip(float fValue);
```

**Parameters:**

- `fValue`: `float fValue`.

**Return Value:**

- Returns a `xge_xui_size_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSizePercent

Provides the `xgeXuiSizePercent` operation.

**Purpose:**

Provides the `xgeXuiSizePercent` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
```

**Parameters:**

- `fValue`: `float fValue`.

**Return Value:**

- Returns a `xge_xui_size_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSizeGrow

Provides the `xgeXuiSizeGrow` operation.

**Purpose:**

Provides the `xgeXuiSizeGrow` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
```

**Parameters:**

- `fValue`: `float fValue`.

**Return Value:**

- Returns a `xge_xui_size_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSizeContent

Provides the `xgeXuiSizeContent` operation.

**Purpose:**

Provides the `xgeXuiSizeContent` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_xui_size_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiInit

Initializes the Xui object or subsystem.

**Purpose:**

Initializes the Xui object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiInit(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiUnit`

---

### xgeXuiUnit

Releases resources associated with Xui.

**Purpose:**

Releases resources associated with Xui. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiUnit(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`
- `xgeXuiReleaseWidgetCapture`

---

### xgeXuiRoot

Provides the `xgeXuiRoot` operation.

**Purpose:**

Provides the `xgeXuiRoot` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns a `xge_xui_widget` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSetDipScale

Sets Xui Dip Scale state or configuration.

**Purpose:**

Sets Xui Dip Scale state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSetDipScale(xge_xui_context pContext, float fScale);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `fScale`: `float fScale`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiGetDipScale

Gets Xui Dip Scale state or information.

**Purpose:**

Gets Xui Dip Scale state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeXuiGetDipScale(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns the requested numeric value. Invalid inputs generally return `0`.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiSetSafeAreaPx

Sets the root safe-area padding in pixels.

**Purpose:**

The XUI root and overlay root automatically follow the window size. This function applies safe-area insets to the root content rect while leaving the overlay root full-window.

**Prototype:**

```c
XGE_API void xgeXuiSetSafeAreaPx(xge_xui_context pContext, float fLeft, float fTop, float fRight, float fBottom);
```

**Parameters:**

- `pContext`: XUI context.
- `fLeft`, `fTop`, `fRight`, `fBottom`: safe-area insets in pixels.

**Return Value:**

- This function does not return a value.

**XSON:**

A page can declare top-level `safeArea`. Page loading applies it to the current context root, and `xgeXuiPageUnload` restores the previous root padding. `safeArea` accepts a number, a two-item `[x, y]` array, a four-item `[left, top, right, bottom]` array, and spacing tokens:

```json
{
  "xui": 1,
  "tokens": { "spacing": { "safeX": 12, "safeY": 24 } },
  "safeArea": [ "@spacing.safeX", "@spacing.safeY" ],
  "tree": { "id": "root" }
}
```

---

### xgeXuiThemeDefault

Provides the `xgeXuiThemeDefault` operation.

**Purpose:**

Provides the `xgeXuiThemeDefault` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiThemeDefault(xge_xui_theme pTheme);
```

**Parameters:**

- `pTheme`: `xge_xui_theme pTheme`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSetTheme

Sets Xui Theme state or configuration.

**Purpose:**

Sets Xui Theme state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme);
XGE_API uint32_t xgeXuiGetThemeVersion(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pTheme`: `const xge_xui_theme_t* pTheme`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiGetTheme

Gets Xui Theme state or information.

**Purpose:**

Gets Xui Theme state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns a handle or borrowed pointer as documented by the API ownership rules. `NULL` indicates failure or absence.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiStyleDefault

Provides the `xgeXuiStyleDefault` operation.

**Purpose:**

Provides the `xgeXuiStyleDefault` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiStyleDefault(xge_xui_style_t* pStyle);
```

**Parameters:**

- `pStyle`: `xge_xui_style_t* pStyle`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiStyleFromTheme

Provides the `xgeXuiStyleFromTheme` operation.

**Purpose:**

Provides the `xgeXuiStyleFromTheme` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme);
```

**Parameters:**

- `pStyle`: `xge_xui_style_t* pStyle`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pTheme`: `const xge_xui_theme_t* pTheme`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiRefreshRequest

Provides the `xgeXuiRefreshRequest` operation.

**Purpose:**

Provides the `xgeXuiRefreshRequest` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiRefreshRequest(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiRefreshNeeded

Provides the `xgeXuiRefreshNeeded` operation.

**Purpose:**

Provides the `xgeXuiRefreshNeeded` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiRefreshNeeded(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiRefreshClear

Clears or resets Xui Refresh Clear state.

**Purpose:**

Clears or resets Xui Refresh Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiRefreshClear(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiLayoutBatchBegin

Begins the Xui Layout Batch Begin operation.

**Purpose:**

Begins the Xui Layout Batch Begin operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLayoutBatchBegin(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiLayoutBatchEnd

Ends the Xui Layout Batch End operation.

**Purpose:**

Ends the Xui Layout Batch End operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLayoutBatchEnd(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSetHost

Sets Xui Host state or configuration.

**Purpose:**

Sets Xui Host state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pHost`: `const xge_xui_host_t* pHost`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiGetHost

Gets Xui Host state or information.

**Purpose:**

Gets Xui Host state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns a handle or borrowed pointer as documented by the API ownership rules. `NULL` indicates failure or absence.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

## API Reference: Page / Loader / Binder

### xgeXuiPageLoad

Loads an XUI page from a resource URI.

```c
XGE_API int xgeXuiPageLoad(xge_xui_context pContext, const char* sURI, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

Resource bytes are read through `xgeResourceLoad`. The current implementation creates a retained widget tree from XSON; regular nodes attach to the active XUI root, while structural overlay controls such as `popup` attach to the overlay root through an overlay portal. Service overlays such as `msgTip`, `msgBox`, `inputBox`, and `toast` are opened by C API callbacks and are not XSON page nodes. It supports `tree.type/id/name/children`, top-level `styles`, widget `style` references, style `@parent` inheritance, and basic layout/size/spacing/alignment/visual inline overrides. Style tables share fields through the XValue table parent chain and are released with the page document. A cyclic `@parent` chain fails page loading and reports `style parent cycle` through `xgeXuiPageGetError`.

The first `tree.type` set supports structural widgets: `panel`, `absolute`, `row`, `column`, `stack`, `grid`, `dock`, and `scrollView`/`scroll`, plus lightweight stateful controls: `button`, `image`, `input`, `label`, and `separator`. Unknown or non-string types fail page loading.

Common widget/style fields include `overflow`, `layer`, `z`, `zIndex`, `hitTestVisible`, `inputTransparent`, `tabStop`, `tabIndex`, `imeMode`, `borderColor`, `borderWidth`, `focusRingColor`, `focusRingWidth`, `disabledOverlay`, `debugOutlineColor`, and `debugOutlineWidth`. Sibling paint, hit-test, and point-event target selection use `layer > z > treeOrder`.

`scrollView`/`scroll` binds an `xge_xui_scroll_view_t` backed by `ScrollModel + ScrollFrame + contentWidget`; XSON `children` attach to the content widget. It supports `contentSize`, `contentWidth`, `contentHeight`, `offset`/`scrollOffset`/`contentOffset`, `scrollX`, `scrollY`, `wheelAxis`, `wheelStep`, `dragMode`/`contentDrag`, `scrollbarPolicy`, `scrollbarPolicyX`, `scrollbarPolicyY`, `scrollbarMode`, `scrollbarSize`, `minThumbSize`, `thumbRadius`, `scrollbarButtonSize`, `backgroundColor`/`background`, `barColor`, and `thumbColor`. The scroll offset is applied to the content widget layout, so normal child controls do not need to translate event coordinates manually. `button` supports `text`, `font`, `textColor`, `textAlign`, `textVAlign`, `color`/`background`, `hoverColor`, `activeColor`, `focusColor`, `disabledColor`, and `onClick`. `button.onClick` uses the button control's own `xgeXuiButtonSetClick` path and does not overwrite the control `pUser`. `image` supports `texture`, `src`, `source`/`srcRect`, `color`/`tint`, and `mode`; `texture` references a C-registered texture token, while `src` is synchronously loaded and owned by the page until unload. `input` supports `text`/`value`, `placeholder`, `font`, `textColor`, `background`/`backgroundColor`, `focusColor`, `cursorColor`, `placeholderColor`, `selectionColor`, `disabledTextColor`, `disabledBackgroundColor`, `password`, `readonly`, `disabled`, and `selection`; text buffers, the default menu, and IME state are owned by `xge_xui_input_t` and released through `xgeXuiInputUnit` during page unload. `label` supports `text`, `font`, `textColor`/`color`, `textAlign`, and `textVAlign`; `font` references a C-registered font token such as `"@fonts.body"`. `separator` supports `orientation`, `thickness`, and `color`/`background`. These controls are stored in a fixed-capacity page control arena and are released through their matching `Unit` functions during `xgeXuiPageUnload`.

The first `imports` implementation imports only `styles`, `tokens`, and `templates` from other XSON resources; it never imports `tree`. URIs with a scheme are passed to `xgeResourceLoad` as-is. Relative paths are resolved against the current XSON URI directory. Imports are merged in array order, later imports override earlier imports, and local declarations in the current XSON override imported declarations.

`tokens.colors` can be referenced by color fields, for example `"background": "@colors.panel"`. `tokens.spacing` can be referenced by size, spacing, and radius fields, for example `"gap": "@spacing.md"`. An unqualified `@name` lookup checks top-level tokens, then `colors`, `spacing`, `fonts`, and `textures`. A missing token fails page loading and reports the field path. C code can register context-level fallback tokens with `xgeXuiTokenSetColor`, `xgeXuiTokenSetSpacing`, `xgeXuiTokenSetFont`, and `xgeXuiTokenSetTexture`; XSON/import tokens take priority. Font and texture tokens only reference externally owned objects.

Size fields support numeric px values, `"content"`, `"grow"`, `"grow:N"`, `"N*"`, `"*"`, `"N%"`, `"Ndip"`, and `{ "unit": "...", "value": N }`. `padding` and `margin` support a number, a two-item `[x, y]` array, or a four-item `[left, top, right, bottom]` array. Top-level `safeArea` uses the same number/array format and restores the previous root padding when the page unloads. `anchor` supports objects such as `{ "left": N, "right": "5%" }`; each present edge enables its anchor flag. Grid supports `grid.columns/rowHeight/columnGap/rowGap/columnSpan` plus top-level aliases `columns/rowHeight/columnGap/rowGap/columnSpan`. DockLayout supports `type:"dock"` or `layout:"dock"` on the container, and child `dock` values `top`, `bottom`, `left`, `right`, `fill`, and `center`. Children consume the remaining rectangle in declaration order; `fill` and `center` use the current remaining rectangle. ScrollView example:

```json
{
  "type": "scrollView",
  "width": 320,
  "height": 240,
  "padding": 8,
  "contentSize": [ 320, 800 ],
  "offset": { "x": 0, "y": 120 },
  "children": [
    { "type": "column", "children": [] }
  ]
}
```

`onClick` binds a widget to a C callback registered with `xgeXuiBinderSetClick`, for example `"onClick": "ok"`. An unregistered event name fails page loading and reports the field path. XSON scripts are not supported; `script` and `onClickScript` are treated as load errors. Structural widgets use the generic page-created event procedure; `button` routes `onClick` through the button control's own click callback. `input.onChange` and `input.onSubmit` are currently rejected instead of being represented by empty stubs; they will be added together with model/value binding.

---

### xgeXuiPageLoadMemory

Loads an XUI page from memory.

```c
XGE_API int xgeXuiPageLoadMemory(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

The memory buffer is copied into a page-owned resource and released by `xgeXuiPageUnload`.

Minimal XSON example:

```json
{
  "xui": 1,
  "imports": [ "shared/styles.xson" ],
  "styles": {
    "base": { "width": 120, "height": "grow:2", "padding": [8, 4] },
    "panel": { "@parent": "base", "layout": "row", "gap": 8 }
  },
  "tree": {
    "type": "column",
    "id": "root",
    "style": "panel",
    "children": [
      { "type": "row", "name": "child", "width": "50%" }
    ]
  }
}
```

---

### xgeXuiPageUnload / Root / Find / GetError

```c
XGE_API void xgeXuiPageUnload(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId);
XGE_API const char* xgeXuiPageGetError(xge_xui_page_t* pPage);
```

`xgeXuiPageUnload` releases the loader-created root, overlay portal widgets, XSON document, imports, merged style/token/template tables, stateful controls stored in the page control arena, and resource. `xgeXuiPageFind` first uses the fixed-capacity id/name index built during page loading; if the index overflows, or if `pRoot` was filled manually without an index, it falls back to a recursive scan under the page root and the page overlay portal registry. A failed page load rolls back loader-created resources, stateful controls, and widget tree while preserving the error string returned by `xgeXuiPageGetError`. `xgeXuiPageStyleVersion` returns the current page style version; in the first implementation a successful page load sets it to `1`.

Changing context-level tokens increments the context theme version, but XSON is not reparsed from the layout or paint hot path. To apply new token values to an already loaded page, call `xgeXuiPageSyncStyle`; it calls `xgeXuiPageRefreshStyle` only when the page's recorded theme version is older than the context version. `xgeXuiPageRefreshStyle` walks the page XSON tree again, resolves layout/visual fields into each widget's `xge_xui_style_t` cache, resolves portaled overlay nodes through the page path registry, increments the page style version, and marks affected widgets layout/paint dirty.

```c
XGE_API int xgeXuiPageRefreshStyle(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageSyncStyle(xge_xui_page_t* pPage);
```

---

### xgeXuiBinderInit / xgeXuiBinderSetClick

```c
XGE_API void xgeXuiBinderInit(xge_xui_binder_t* pBinder);
XGE_API int xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser);
```

Binder is a lightweight registry from XSON event names to C callbacks. XSON parsing will use these entries for `onClick`.

---

### XSON Style Lifetime Rules

`styles`, imported `styles`, and the loader-created merged style table are owned by `xge_xui_page_t`. `xvoTableSetParent` only stores a raw parent table pointer; it does not add a reference and does not take ownership. In the first implementation, every XValue table that can participate in `@parent` lookup must share the page lifetime and must be released only by `xgeXuiPageUnload`.

---

### xgeXuiWidgetCreate

Creates a Xui Widget resource.

**Purpose:**

Creates a Xui Widget resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_xui_widget` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`
- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetFree

Releases resources associated with Xui Widget.

**Purpose:**

Releases resources associated with Xui Widget. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`
- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetAdd

Adds a child widget to the end of a parent widget's child list.

**Purpose:**

This is the public user-children entry point. Container, Viewport, and Overlay widgets may own children. Control widgets reject user children.

**Prototype:**

```c
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
```

**Parameters:**

- `pParent`: `xge_xui_widget pParent`.
- `pChild`: `xge_xui_widget pChild`.

**Return Value:**

- Returns `XGE_OK` on success.
- Returns `XGE_ERROR_INVALID_ARGUMENT` for invalid arguments, cycles, or a parent with role `XGE_XUI_WIDGET_ROLE_CONTROL`.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- If `pChild` already has a parent, it is removed from that parent before being attached.
- Adding a child marks the parent layout and paint dirty.
- Internal slots used by built-in controls are owned by those controls and are not exposed through this user-children API.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetRemove

Gets Xui Widget Remove state or information.

**Purpose:**

Gets Xui Widget Remove state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetSetId

Sets Xui Widget Id state or configuration.

**Purpose:**

Sets Xui Widget Id state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iId`: `int iId`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetGetId`
- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetGetId

Gets Xui Widget Id state or information.

**Purpose:**

Gets Xui Widget Id state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetId(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetSetName

Sets Xui Widget Name state or configuration.

**Purpose:**

Sets Xui Widget Name state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetGetName

Gets Xui Widget Name state or information.

**Purpose:**

Gets Xui Widget Name state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const char* xgeXuiWidgetGetName(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns a borrowed UTF-8 string pointer. The caller must not free it.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetFindById

Gets Xui Widget Find By Id state or information.

**Purpose:**

Gets Xui Widget Find By Id state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId);
```

**Parameters:**

- `pRoot`: `xge_xui_widget pRoot`.
- `iId`: `int iId`.

**Return Value:**

- Returns a `xge_xui_widget` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetFindByName

Gets Xui Widget Find By Name state or information.

**Purpose:**

Gets Xui Widget Find By Name state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName);
```

**Parameters:**

- `pRoot`: `xge_xui_widget pRoot`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns a `xge_xui_widget` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetSetRect

Sets Xui Widget Rect state or configuration.

**Purpose:**

Sets Xui Widget Rect state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `tRect`: `xge_rect_t tRect`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetGetRect

Gets the widget border rectangle.

**Purpose:**

Gets the current `borderRect`. Use `xgeXuiWidgetGetOuterRect`, `xgeXuiWidgetGetBorderRect`, `xgeXuiWidgetGetPaddingRect`, or `xgeXuiWidgetGetContentRect` when code needs an explicit box-model layer.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the widget `borderRect`, or a zero rectangle when the widget is invalid.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Use `xgeXuiWidgetGetOuterRect`, `xgeXuiWidgetGetBorderRect`, `xgeXuiWidgetGetPaddingRect`, or `xgeXuiWidgetGetContentRect` when code needs an explicit box-model layer.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetGetOuterRect

Gets the widget `outerRect`, including margin.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetOuterRect(xge_xui_widget pWidget);
```

Returns a zero rectangle when the widget is invalid.

---

### xgeXuiWidgetGetBorderRect

Gets the widget `borderRect`. This currently matches `xgeXuiWidgetGetRect`.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetBorderRect(xge_xui_widget pWidget);
```

Returns a zero rectangle when the widget is invalid.

---

### xgeXuiWidgetGetPaddingRect

Gets the widget `paddingRect`. When border width is set, `paddingRect` is inset from `borderRect` by that width.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetPaddingRect(xge_xui_widget pWidget);
```

Returns a zero rectangle when the widget is invalid.

---

### xgeXuiWidgetGetContentRect

Gets the widget `contentRect`, the padding-adjusted area used by child layout, content clipping, and scroll viewports.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetContentRect(xge_xui_widget pWidget);
```

Returns a zero rectangle when the widget is invalid.

---

### xgeXuiWidgetSetStyle

Sets Xui Widget Style state or configuration.

**Purpose:**

Sets Xui Widget Style state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pStyle`: `const xge_xui_style_t* pStyle`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetGetStyle

Gets Xui Widget Style state or information.

**Purpose:**

Gets Xui Widget Style state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns a handle or borrowed pointer as documented by the API ownership rules. `NULL` indicates failure or absence.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetLayout

Sets Xui Widget Layout state or configuration.

**Purpose:**

Sets Xui Widget Layout state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iLayout`: `int iLayout`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetDock

Sets the child's dock side for a parent DockLayout container.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetDock(xge_xui_widget pWidget, int iDock);
XGE_API int xgeXuiWidgetGetDock(xge_xui_widget pWidget);
```

**Notes:**

- `iDock` accepts `XGE_XUI_DOCK_TOP`, `XGE_XUI_DOCK_BOTTOM`, `XGE_XUI_DOCK_LEFT`, `XGE_XUI_DOCK_RIGHT`, and `XGE_XUI_DOCK_FILL`.
- `XGE_XUI_DOCK_CENTER` is an alias of `XGE_XUI_DOCK_FILL`.
- The field affects layout only when the parent uses `XGE_XUI_LAYOUT_DOCK`.

---

### xgeXuiWidgetSetSize

Sets Xui Widget Size state or configuration.

**Purpose:**

Sets Xui Widget Size state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `tWidth`: `xge_xui_size_t tWidth`.
- `tHeight`: `xge_xui_size_t tHeight`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetMinSize

Sets Xui Widget Min Size state or configuration.

**Purpose:**

Sets Xui Widget Min Size state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `tWidth`: `xge_xui_size_t tWidth`.
- `tHeight`: `xge_xui_size_t tHeight`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetMaxSize

Sets Xui Widget Max Size state or configuration.

**Purpose:**

Sets Xui Widget Max Size state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `tWidth`: `xge_xui_size_t tWidth`.
- `tHeight`: `xge_xui_size_t tHeight`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetGrid

Sets Xui Widget Grid state or configuration.

**Purpose:**

Sets Xui Widget Grid state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iColumns`: `int iColumns`.
- `fRowHeight`: row height. Values less than or equal to 0 use the cell width, producing square grid cells.
- `fColumnGap`: `float fColumnGap`.
- `fRowGap`: `float fRowGap`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`
- `xgeXuiWidgetSetGridColumnSpan`

---

### xgeXuiWidgetSetGridColumnSpan

Sets the number of grid columns occupied by a child widget.

**Purpose:**

Use this on a child of a grid container to span multiple columns for headers, wide buttons, or card titles.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetGridColumnSpan(xge_xui_widget pWidget, int iColumnSpan);
```

**Parameters:**

- `pWidget`: child widget in a grid container.
- `iColumnSpan`: number of columns to occupy. Values less than or equal to 1 are treated as 1; values larger than the parent grid column count are clamped by layout.

**Return Value:**

- This function does not return a value.

**Notes:**

- The first version supports column span only, not row span or a CSS Grid-style occupancy matrix.
- If the current row does not have enough remaining columns, the widget is moved to the next row.

**Example:**

```c
xgeXuiWidgetSetGridColumnSpan(title, 2);
```

**Related APIs:**

- `xgeXuiWidgetSetGrid`

---

### xgeXuiWidgetSetAlign

Sets Xui Widget Align state or configuration.

**Purpose:**

Sets Xui Widget Align state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iAlignX`: `int iAlignX`.
- `iAlignY`: `int iAlignY`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetZ

Sets the widget z value.

**Purpose:**

The z value participates in sibling paint, hit-test, and point-event target ordering.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iZ`: `int iZ`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Sibling ordering is `layer > z > treeOrder`.
- The function marks the parent paint dirty, or the widget itself when it has no parent.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetGetZ`
- `xgeXuiWidgetSetLayer`
- `xgeXuiWidgetGetTreeOrder`

---

### xgeXuiWidgetGetZ

Gets the widget z value.

**Purpose:**

The returned value is the z component of `layer > z > treeOrder` sibling ordering.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the z value for a valid widget.
- Returns 0 for an invalid widget.

**Related APIs:**

- `xgeXuiWidgetSetZ`
- `xgeXuiWidgetGetLayer`
- `xgeXuiWidgetGetTreeOrder`

---

### xgeXuiWidgetSetLayer

Sets the widget layer.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetLayer(xge_xui_widget pWidget, int iLayer);
```

Valid values are `XGE_XUI_LAYER_NORMAL`, `XGE_XUI_LAYER_FLOATING`, `XGE_XUI_LAYER_POPUP`, `XGE_XUI_LAYER_MODAL`, `XGE_XUI_LAYER_TOOLTIP`, `XGE_XUI_LAYER_DRAG_ADORNER`, and `XGE_XUI_LAYER_DEBUG`. Invalid values fall back to `NORMAL`.

---

### xgeXuiWidgetGetLayer

Gets the widget layer.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetLayer(xge_xui_widget pWidget);
```

Invalid widgets return `XGE_XUI_LAYER_NORMAL`.

---

### xgeXuiWidgetGetTreeOrder

Gets the widget treeOrder value.

**Prototype:**

```c
XGE_API uint32_t xgeXuiWidgetGetTreeOrder(xge_xui_widget pWidget);
```

`xgeXuiWidgetAdd` assigns treeOrder. It is used as the stable tie-breaker when siblings have the same layer and z value.

---

### xgeXuiWidgetSetOverflow

Sets the widget overflow policy.

**Purpose:**

`overflow` is a widget foundation policy. Valid values are `XGE_XUI_OVERFLOW_VISIBLE`, `XGE_XUI_OVERFLOW_CLIP`, `XGE_XUI_OVERFLOW_HIDDEN`, and `XGE_XUI_OVERFLOW_SCROLL`. `clip`, `hidden`, and `scroll` enable clipping to the current content rect. `scroll` marks explicit scroll-view style widgets; it does not auto-create scrollbars for ordinary widgets.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetOverflow(xge_xui_widget pWidget, int iOverflow);
```

**Related APIs:**

- `xgeXuiWidgetGetOverflow`
- `xgeXuiWidgetSetClip`

---

### xgeXuiWidgetGetOverflow

Gets the widget overflow policy.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetOverflow(xge_xui_widget pWidget);
```

Invalid widgets return `XGE_XUI_OVERFLOW_VISIBLE`.

---

### xgeXuiWidgetSetAnchorPx

Sets Xui Widget Anchor Px state or configuration.

**Purpose:**

Sets Xui Widget Anchor Px state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iAnchor`: `int iAnchor`.
- `fLeft`: `float fLeft`.
- `fTop`: `float fTop`.
- `fRight`: `float fRight`.
- `fBottom`: `float fBottom`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetGetAnchor

Gets Xui Widget Anchor state or information.

**Purpose:**

Gets Xui Widget Anchor state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetMarginPx

Sets Xui Widget Margin Px state or configuration.

**Purpose:**

Sets Xui Widget Margin Px state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `fLeft`: `float fLeft`.
- `fTop`: `float fTop`.
- `fRight`: `float fRight`.
- `fBottom`: `float fBottom`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetPaddingPx

Sets Xui Widget Padding Px state or configuration.

**Purpose:**

Sets Xui Widget Padding Px state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `fLeft`: `float fLeft`.
- `fTop`: `float fTop`.
- `fRight`: `float fRight`.
- `fBottom`: `float fBottom`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetBackground

Sets Xui Widget Background state or configuration.

**Purpose:**

Sets Xui Widget Background state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetRadius

Sets Xui Widget Radius state or configuration.

**Purpose:**

Sets Xui Widget Radius state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `fRadius`: `float fRadius`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetBorder

Sets the widget base border.

**Purpose:**

Sets the border width and color painted by the widget base layer. The border width participates in the box model: `contentRect` is inset from `borderRect` by border width plus padding.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetBorder(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**Parameters:**

- `pWidget`: widget object.
- `fWidth`: border width. Values below 0 are clamped to 0.
- `iColor`: border color. Alpha 0 disables drawing.

**Return Value:**

- This function does not return a value.

**Ownership:**

Ownership remains with the caller.

**Notes:**

- Marks layout and paint dirty.

**Example:**

```c
xgeXuiWidgetSetBorder(panel, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
```

**Related APIs:**

- `xgeXuiWidgetSetBackground`
- `xgeXuiWidgetSetFocusRing`

---

### xgeXuiWidgetSetFocusRing

Sets the widget base focus ring.

**Purpose:**

Sets the focus ring automatically painted by the widget base layer when the widget is focused. The default width is 0, so existing controls keep their custom focus visuals unless this is explicitly enabled.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetFocusRing(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**Parameters:**

- `pWidget`: widget object.
- `fWidth`: focus ring width. Values below 0 are clamped to 0.
- `iColor`: focus ring color. Alpha 0 disables drawing.

**Return Value:**

- This function does not return a value.

**Ownership:**

Ownership remains with the caller.

**Notes:**

- Marks paint dirty. Only the currently focused widget paints the focus ring.

**Example:**

```c
xgeXuiWidgetSetFocusRing(input, 1.5f, XGE_COLOR_RGBA(53, 174, 234, 255));
```

**Related APIs:**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiSetFocus`

---

### xgeXuiWidgetSetDisabledOverlay

Sets the widget disabled overlay.

**Purpose:**

Sets the translucent overlay automatically painted by the widget base layer when the widget is disabled. The default color is transparent, so existing control visuals are unchanged.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetDisabledOverlay(xge_xui_widget pWidget, uint32_t iColor);
```

**Parameters:**

- `pWidget`: widget object.
- `iColor`: disabled overlay color. Alpha 0 disables drawing.

**Return Value:**

- This function does not return a value.

**Ownership:**

Ownership remains with the caller.

**Notes:**

- The overlay is painted after widget content and children, before border/debug outline.

**Example:**

```c
xgeXuiWidgetSetDisabledOverlay(panel, XGE_COLOR_RGBA(237, 245, 250, 160));
```

**Related APIs:**

- `xgeXuiWidgetSetEnabled`
- `xgeXuiWidgetSetDebugOutline`

---

### xgeXuiWidgetSetDebugOutline

Sets the widget debug outline.

**Purpose:**

Sets an always-painted debug outline for a single widget, useful for temporary layout and clipping inspection. The default width is 0.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetDebugOutline(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**Parameters:**

- `pWidget`: widget object.
- `fWidth`: outline width. Values below 0 are clamped to 0.
- `iColor`: outline color. Alpha 0 disables drawing.

**Return Value:**

- This function does not return a value.

**Ownership:**

Ownership remains with the caller.

**Notes:**

- The debug outline is painted after background, content, disabled overlay, border, and focus ring.

**Example:**

```c
xgeXuiWidgetSetDebugOutline(panel, 1.0f, XGE_COLOR_RGBA(255, 80, 80, 220));
```

**Related APIs:**

- `xgedbgXuiDebugOverlayPaint`
- `xgeXuiWidgetSetDisabledOverlay`

---

### xgeXuiWidgetGetFlags

Gets widget flag state, including visibility, enabled state, focusability, clipping, hit-test visibility, input transparency, and dirty flags.

**Purpose:**

Gets widget flag state, including visibility, enabled state, focusability, clipping, hit-test visibility, input transparency, and dirty flags. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns a `uint32_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetVisible

Sets Xui Widget Visible state or configuration.

**Purpose:**

Sets Xui Widget Visible state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `bVisible`: `int bVisible`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetEnabled

Sets Xui Widget Enabled state or configuration.

**Purpose:**

Sets Xui Widget Enabled state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `bEnabled`: `int bEnabled`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetHitTestVisible

Sets whether the widget participates in hit testing.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetHitTestVisible(xge_xui_widget pWidget, int bVisible);
```

When `bVisible` is 0, the widget and its subtree cannot become hit targets. The default is 1.

---

### xgeXuiWidgetIsHitTestVisible

Gets whether the widget participates in hit testing.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsHitTestVisible(xge_xui_widget pWidget);
```

Invalid widgets return 0.

---

### xgeXuiWidgetSetInputTransparent

Sets whether the widget itself is transparent to input.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetInputTransparent(xge_xui_widget pWidget, int bTransparent);
```

When `bTransparent` is non-zero, the widget itself cannot become the hit target, but its children can still be hit. If no child is hit, input passes through to lower siblings. The default is 0.

---

### xgeXuiWidgetIsInputTransparent

Gets whether the widget itself is transparent to input.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsInputTransparent(xge_xui_widget pWidget);
```

Invalid widgets return 0.

---

### xgeXuiWidgetSetFocusable

Sets Xui Widget Focusable state or configuration.

**Purpose:**

Sets Xui Widget Focusable state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `bFocusable`: `int bFocusable`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- A focusable widget is tab-stoppable by default. Call `xgeXuiWidgetSetTabStop(widget, 0)` when the widget should still be focusable by mouse/code but skipped by TAB traversal.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetTabStop

Sets whether a widget participates in TAB traversal.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetTabStop(xge_xui_widget pWidget, int bTabStop);
```

`tabStop` does not affect manual `xgeXuiSetFocus`; it only affects TAB traversal. The widget must still be visible, enabled, and focusable to be selected by TAB.

TAB traversal uses the page root as its default scope. When a visible modal overlay exists, traversal is limited to the top modal subtree. A non-modal overlay scopes TAB only while the current focus is already inside that overlay, so opening a popup does not steal root TAB traversal. Regular widgets can declare a named focus scope with `xgeXuiWidgetSetFocusScope`; while current focus is inside that subtree, TAB traversal is limited to the nearest visible enabled scope.

---

### xgeXuiWidgetIsTabStop

Returns whether a widget can currently be reached by TAB traversal.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsTabStop(xge_xui_widget pWidget);
```

Invalid widgets and non-focusable widgets return 0.

---

### xgeXuiWidgetSetTabIndex

Sets the TAB traversal order value.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetTabIndex(xge_xui_widget pWidget, int iTabIndex);
```

TAB traversal uses ascending `tabIndex`, then ascending `treeOrder`. The default `tabIndex` is 0.

---

### xgeXuiWidgetGetTabIndex

Gets the TAB traversal order value.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetTabIndex(xge_xui_widget pWidget);
```

Invalid widgets return 0.

---

### xgeXuiWidgetSetFocusScope

Sets whether the widget is a named focus scope.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetFocusScope(xge_xui_widget pWidget, int bFocusScope);
```

A focus scope limits keyboard TAB traversal and default/cancel action lookup. It does not affect hit testing, event routing, or manual `xgeXuiSetFocus`. When current focus is inside a focus-scope subtree, FocusManager uses the nearest visible enabled focus scope; otherwise it falls back to the page root. Visible modal overlays and non-modal overlays containing current focus take priority over named focus scopes.

---

### xgeXuiWidgetIsFocusScope

Returns whether the widget is declared as a named focus scope.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsFocusScope(xge_xui_widget pWidget);
```

Returns non-zero when the widget has the focus-scope declaration. This reports the declaration only; whether the scope is active also depends on current focus, the visible/enabled parent chain, and overlay/modal scope priority.

---

### xgeXuiWidgetSetDefaultAction

Sets the widget Enter default action.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetDefaultAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
```

Passing NULL for `procAction` clears the default action. When Enter is pressed, normal event routing does not consume the event, and no pointer capture is active, FocusManager searches the current focus scope in TAB order and invokes the first visible enabled default action. A visible modal overlay constrains the search scope; a non-modal overlay constrains it only while current focus is inside that overlay subtree. Named focus scopes apply after overlay/modal scopes.

---

### xgeXuiWidgetSetCancelAction

Sets the widget Escape cancel action.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetCancelAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
```

Passing NULL for `procAction` clears the cancel action. When Escape is pressed, normal event routing does not consume the event, and no pointer capture is active, FocusManager searches the current focus scope in TAB order and invokes the first visible enabled cancel action. If pointer capture is active, Escape first releases capture and sends `XGE_EVENT_XUI_CAPTURE_LOST` followed by `XGE_EVENT_XUI_CAPTURE_CANCEL`. Named focus scopes apply after overlay/modal scopes.

---

### xgeXuiWidgetSetImeMode

Sets the widget IME request policy.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetImeMode(xge_xui_widget pWidget, int iImeMode);
```

Supported values are `XGE_XUI_IME_DISABLED`, `XGE_XUI_IME_ENABLED`, and `XGE_XUI_IME_AUTO`. The default is disabled. When focus changes, the XUI context synchronizes the system IME through `xgeImeSetEnabled` and refreshes the focused widget's candidate rect; widgets that do not request IME keep it disabled and clear the candidate rect.

---

### xgeXuiWidgetGetImeMode

Gets the widget IME request policy.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetImeMode(xge_xui_widget pWidget);
```

Invalid widgets return `XGE_XUI_IME_DISABLED`. `Input` and `TextEdit` default to enabled; password input switches to disabled unless explicitly overridden by the caller.

---

### xgeXuiWidgetSetImeCandidateRect

Registers an IME candidate rect resolver for a widget.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetImeCandidateRect(xge_xui_widget pWidget, xge_xui_ime_candidate_rect_proc procRect, void* pUser);
```

`procRect` returns the rectangle where the system IME candidate UI should anchor. Passing `NULL` clears the custom resolver. Widgets without a resolver use `contentRect`, falling back to `borderRect` / `rect` when needed. If the widget is focused, the context cache is refreshed immediately.

---

### xgeXuiWidgetGetImeCandidateRect

Gets the candidate rect for a specific widget.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetImeCandidateRect(xge_xui_widget pWidget);
```

This resolves only the widget rectangle; it does not check whether the widget is currently requesting IME. Platform backends should use `xgeXuiGetImeCandidateRect` for the focused widget.

---

### xgeXuiHasImeCandidateRect / xgeXuiGetImeCandidateRect

Gets the IME candidate rect for the currently focused widget in a context.

**Prototype:**

```c
XGE_API int xgeXuiHasImeCandidateRect(xge_xui_context pContext);
XGE_API xge_rect_t xgeXuiGetImeCandidateRect(xge_xui_context pContext);
```

`xgeXuiHasImeCandidateRect` returns 1 only when the focused widget requests IME. `xgeXuiGetImeCandidateRect` refreshes the cache on demand and returns a zero rectangle when no candidate rect is active. `Input` and `TextEdit` register cursor-based resolvers during initialization.

---

### xgeXuiWidgetSetClip

Sets Xui Widget Clip state or configuration.

**Purpose:**

Convenience API equivalent to switching between `XGE_XUI_OVERFLOW_VISIBLE` and `XGE_XUI_OVERFLOW_CLIP`. New code should prefer `xgeXuiWidgetSetOverflow`.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `bClip`: `int bClip`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- When clip is disabled, children may overflow visually. `overflow: visible` also allows overflowing children to be hit outside the parent border.
- When clip is enabled and the point is inside the widget rect but outside `tContentRect`, hit testing returns the clipped widget itself instead of an overflowing child.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetCaptureEvent

Sets Xui Widget Capture Event state or configuration.

**Purpose:**

Sets Xui Widget Capture Event state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `procEvent`: `xge_xui_event_proc procEvent`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetMeasure

Sets Xui Widget Measure state or configuration.

**Purpose:**

Sets Xui Widget Measure state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `procMeasure`: `xge_xui_measure_proc procMeasure`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetSetPaintBefore

Sets a paint callback that runs before widget base paint.

**Purpose:**

Use it for underlay drawing such as a backdrop. The callback runs before the widget background, content clip, and normal `procPaint`.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetPaintBefore(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `procPaint`: `xge_xui_paint_proc procPaint`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

The function stores only the callback and borrowed `pUser` pointer.

**Notes:**

- The callback is constrained by ancestor clips, but it is not clipped by the current widget content rect.

---

### xgeXuiWidgetSetPaint

Sets Xui Widget Paint state or configuration.

**Purpose:**

Sets Xui Widget Paint state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `procPaint`: `xge_xui_paint_proc procPaint`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`

---

### xgeXuiWidgetIsVisible

Gets Xui Widget Is Visible state or information.

**Purpose:**

Gets Xui Widget Is Visible state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetIsEnabled

Gets Xui Widget Is Enabled state or information.

**Purpose:**

Gets Xui Widget Is Enabled state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetIsFocusable

Gets Xui Widget Is Focusable state or information.

**Purpose:**

Gets Xui Widget Is Focusable state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- Returns non-zero only when the widget is focusable and its visible/enabled ancestor chain is valid.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetMarkLayout

Gets Xui Widget Mark Layout state or information.

**Purpose:**

Gets Xui Widget Mark Layout state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiWidgetMarkPaint

Gets Xui Widget Mark Paint state or information.

**Purpose:**

Gets Xui Widget Mark Paint state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkStyle(xge_xui_widget pWidget);
```

`xgeXuiWidgetMarkStyle` marks the widget style cache dirty, increments the widget style version, and also marks layout and paint dirty. The first XSON implementation uses `xge_xui_style_t` itself as the lightweight layout/visual style cache after parsing high-frequency fields.

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiWidgetMarkLayout`

---

### xgeXuiHitTest

Provides the `xgeXuiHitTest` operation.

**Purpose:**

Provides the `xgeXuiHitTest` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `fX`: `float fX`.
- `fY`: `float fY`.

**Return Value:**

- Returns a `xge_xui_widget` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiSetFocus

Sets Xui Focus state or configuration.

**Purpose:**

Sets Xui Focus state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- Non-focusable widgets and widgets from another context are ignored. `tabStop` does not affect manual focus; it only affects TAB traversal.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiSetCapture

Sets the pointer 0 capture widget.

**Purpose:**

Captures mouse or legacy pointer events for a widget. Use `xgeXuiSetPointerCapture` when a control needs a specific pointer id.

**Prototype:**

```c
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- `pWidget` must belong to the same context and must be visible and enabled; otherwise the call leaves the current capture unchanged.
- Passing `NULL` is an explicit release and does not emit `XGE_EVENT_XUI_CAPTURE_LOST` or `XGE_EVENT_XUI_CAPTURE_CANCEL`. Replacing the capture widget, hiding, disabling, removing, freeing, or canceling the captured widget releases capture automatically and sends `XGE_EVENT_XUI_CAPTURE_LOST` followed by `XGE_EVENT_XUI_CAPTURE_CANCEL` to the previous capture widget.
- This is equivalent to `xgeXuiSetPointerCapture(pContext, 0, pWidget)`.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiSetPointerCapture

Sets the capture widget for a specific pointer id.

**Prototype:**

```c
XGE_API void xgeXuiSetPointerCapture(xge_xui_context pContext, uint64_t iPointerId, xge_xui_widget pWidget);
```

Pointer id `0` updates the legacy `pCapture` field. Non-zero pointer ids are stored in the context capture table. Passing `NULL` explicitly releases that pointer without lost/cancel notifications. Replacement, hiding, disabling, removing, freeing, and Escape release send `XGE_EVENT_XUI_CAPTURE_LOST` followed by `XGE_EVENT_XUI_CAPTURE_CANCEL`; the event copy carries the released `iPointerId`.

---

### xgeXuiGetPointerCapture

Returns the capture widget for a pointer id, or `NULL`.

```c
XGE_API xge_xui_widget xgeXuiGetPointerCapture(xge_xui_context pContext, uint64_t iPointerId);
```

---

### xgeXuiHasCapture

Returns whether the context has any active pointer capture.

```c
XGE_API int xgeXuiHasCapture(xge_xui_context pContext);
```

---

### xgeXuiWidgetHasCapture

Returns whether a widget owns any active pointer capture.

```c
XGE_API int xgeXuiWidgetHasCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

---

### xgeXuiReleaseWidgetCapture

Explicitly releases every pointer capture currently owned by a widget.

```c
XGE_API void xgeXuiReleaseWidgetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

This API is intended for control `Unit` cleanup, pre-destroy cleanup, and explicit drag cancellation. It releases pointer 0 and non-zero pointer captures owned by `pWidget` without emitting `XGE_EVENT_XUI_CAPTURE_LOST` or `XGE_EVENT_XUI_CAPTURE_CANCEL`. Replacement, hiding, disabling, removing, freeing, and Escape still use the core lost/cancel capture path.

---

### xgeXuiDispatchEvent

Processes or dispatches events for Xui Dispatch Event.

**Purpose:**

Processes or dispatches events for Xui Dispatch Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns `XGE_XUI_EVENT_CONTINUE` when no callback handles the event.
- Returns `XGE_XUI_EVENT_HANDLED` when at least one callback handled the event and no callback consumed it.
- Returns `XGE_XUI_EVENT_CONSUMED` when a callback or an internal policy consumes the event.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- Point events first look up pointer capture by `pEvent->iPointerId`; without capture they use hit testing. Non-point events use legacy pointer 0 capture or focus, falling back to the root widget. Dispatch first calls `procCaptureEvent` from root to target, then the target `procEvent`, then parent `procEvent` callbacks while bubbling back to the root.
- `XGE_XUI_EVENT_HANDLED` continues the current route but prevents XGE fallback and default/cancel actions. `XGE_XUI_EVENT_CONSUMED` stops the current route immediately.
- Widget callbacks receive an event copy with XUI route metadata filled in: `iPointerId`, `iXuiPhase`, `pXuiOriginalTarget`, `pXuiCurrentTarget`, `pXuiCapture`, and `bXuiCaptured`. The caller-owned input event is not modified.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiDispatchQueuedEvents`

---

### xgeXuiEventPush

Processes or dispatches events for Xui Event Push.

**Purpose:**

Processes or dispatches events for Xui Event Push. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiEventPop

Processes or dispatches events for Xui Event Pop.

**Purpose:**

Processes or dispatches events for Xui Event Pop. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `pEvent`: `xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiEventCount

Processes or dispatches events for Xui Event Count.

**Purpose:**

Processes or dispatches events for Xui Event Count. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiEventCount(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiDispatchQueuedEvents

Processes or dispatches events for Xui Dispatch Queued Events.

**Purpose:**

Processes or dispatches events for Xui Dispatch Queued Events. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDispatchQueuedEvents(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiUpdate

Updates Xui Update state.

**Purpose:**

Updates Xui Update state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.
- `fDelta`: `float fDelta`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiPaint

Draws or paints using Xui Paint.

**Purpose:**

Draws or paints using Xui Paint. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiPaint(xge_xui_context pContext);
```

**Parameters:**

- `pContext`: `xge_xui_context pContext`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSizePx`
- `xgeXuiSizeDip`
- `xgeXuiSizePercent`
- `xgeXuiSizeGrow`
- `xgeXuiSizeContent`
- `xgeXuiInit`

---

### xgeXuiTextInit

Initializes the Xui Text object or subsystem.

**Purpose:**

Initializes the Xui Text object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextInit(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`
- `xgeXuiTextDeleteForward`

---

### xgeXuiTextUnit

Releases resources associated with Xui Text.

**Purpose:**

Releases resources associated with Xui Text. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`
- `xgeXuiTextDeleteForward`

---

### xgeXuiTextSet

Sets Xui Text Set state or configuration.

**Purpose:**

Sets Xui Text Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`
- `xgeXuiTextDeleteForward`

---

### xgeXuiTextInsert

Provides the `xgeXuiTextInsert` operation.

**Purpose:**

Provides the `xgeXuiTextInsert` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInsertCodepoint`

---

### xgeXuiTextInsertCodepoint

Provides the `xgeXuiTextInsertCodepoint` operation.

**Purpose:**

Provides the `xgeXuiTextInsertCodepoint` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `iCodepoint`: `uint32_t iCodepoint`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiTextDeleteBack

Provides the `xgeXuiTextDeleteBack` operation.

**Purpose:**

Provides the `xgeXuiTextDeleteBack` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiTextDeleteForward

Provides the `xgeXuiTextDeleteForward` operation.

**Purpose:**

Provides the `xgeXuiTextDeleteForward` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextDeleteForward(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiTextSetCursor

Sets Xui Text Cursor state or configuration.

**Purpose:**

Sets Xui Text Cursor state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `iCursor`: `int iCursor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextGetCursor

Gets Xui Text Cursor state or information.

**Purpose:**

Gets Xui Text Cursor state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextGetCursor(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextSetSelection

Sets Xui Text Selection state or configuration.

**Purpose:**

Sets Xui Text Selection state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `iStart`: `int iStart`.
- `iEnd`: `int iEnd`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextGetSelection

Gets Xui Text Selection state or information.

**Purpose:**

Gets Xui Text Selection state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `pStart`: `int* pStart`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pEnd`: `int* pEnd`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextSetComposition

Sets Xui Text Composition state or configuration.

**Purpose:**

Sets Xui Text Composition state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextClearComposition

Clears or resets Xui Text Clear Composition state.

**Purpose:**

Clears or resets Xui Text Clear Composition state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTextClearComposition(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiTextGetComposition

Gets Xui Text Composition state or information.

**Purpose:**

Gets Xui Text Composition state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const char* xgeXuiTextGetComposition(xge_xui_text pText);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.

**Return Value:**

- Returns a borrowed UTF-8 string pointer. The caller must not free it.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextInputEvent

Processes or dispatches events for Xui Text Input Event.

**Purpose:**

Processes or dispatches events for Xui Text Input Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent);
```

**Parameters:**

- `pText`: `xge_xui_text pText`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiButtonInit

Initializes the Xui Button object or subsystem.

**Purpose:**

Initializes the Xui Button object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `pContext`: `xge_xui_context pContext`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonUnit

Releases resources associated with Xui Button.

**Purpose:**

Releases resources associated with Xui Button. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonUnit(xge_xui_button pButton);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonSetClick

Sets Xui Button Click state or configuration.

**Purpose:**

Sets Xui Button Click state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `procClick`: `xge_xui_click_proc procClick`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonSetText

Sets Xui Button Text state or configuration.

**Purpose:**

Sets Xui Button Text state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonSetTextColor

Sets Xui Button Text Color state or configuration.

**Purpose:**

Sets Xui Button Text Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonSetColors

Sets Xui Button Colors state or configuration.

**Purpose:**

Sets Xui Button Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `iNormal`: `uint32_t iNormal`.
- `iHover`: `uint32_t iHover`.
- `iActive`: `uint32_t iActive`.
- `iFocus`: `uint32_t iFocus`.
- `iDisabled`: `uint32_t iDisabled`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonGetState`

---

### xgeXuiButtonGetState

Gets Xui Button State state or information.

**Purpose:**

Gets Xui Button State state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiButtonGetState(xge_xui_button pButton);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`

---

### xgeXuiButtonEvent

Processes or dispatches events for Xui Button Event.

**Purpose:**

Processes or dispatches events for Xui Button Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent);
```

**Parameters:**

- `pButton`: `xge_xui_button pButton`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`

---

### xgeXuiButtonEventProc

Processes or dispatches events for Xui Button Event Proc.

**Purpose:**

Processes or dispatches events for Xui Button Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`

---

### xgeXuiButtonPaintProc

Draws or paints using Xui Button Paint Proc.

**Purpose:**

Draws or paints using Xui Button Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`

---

### xgeXuiLabelInit

Initializes the Xui Label object or subsystem.

**Purpose:**

Initializes the Xui Label object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.
- `pWidget`: `xge_xui_widget pWidget`.
- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelUnit

Releases resources associated with Xui Label.

**Purpose:**

Releases resources associated with Xui Label. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetText

Sets Xui Label Text state or configuration.

**Purpose:**

Sets Xui Label Text state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetFont

Sets Xui Label Font state or configuration.

**Purpose:**

Sets Xui Label Font state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.
- `pFont`: `xge_font pFont`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetColor

Sets Xui Label Color state or configuration.

**Purpose:**

Sets Xui Label Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetAlign

Sets Xui Label Align state or configuration.

**Purpose:**

Sets Xui Label Align state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.
- `iTextFlags`: `uint32_t iTextFlags`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelMeasure

Provides the `xgeXuiLabelMeasure` operation.

**Purpose:**

Provides the `xgeXuiLabelMeasure` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
```

**Parameters:**

- `pLabel`: `xge_xui_label pLabel`.

**Return Value:**

- Returns a `xge_vec2_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelMeasureProc`

---

### xgeXuiLabelMeasureProc

Provides the `xgeXuiLabelMeasureProc` operation.

**Purpose:**

Provides the `xgeXuiLabelMeasureProc` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns a `xge_vec2_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiLabelPaintProc

Draws or paints using Xui Label Paint Proc.

**Purpose:**

Draws or paints using Xui Label Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`

---

### xgeXuiImageInit

Initializes the Xui Image object or subsystem.

**Purpose:**

Initializes the Xui Image object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.
- `pWidget`: `xge_xui_widget pWidget`.
- `pTexture`: `xge_texture pTexture`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageUnit

Releases resources associated with Xui Image.

**Purpose:**

Releases resources associated with Xui Image. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetTexture

Sets Xui Image Texture state or configuration.

**Purpose:**

Sets Xui Image Texture state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.
- `pTexture`: `xge_texture pTexture`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetSource

Sets Xui Image Source state or configuration.

**Purpose:**

Sets Xui Image Source state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.
- `tSrc`: `xge_rect_t tSrc`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetColor

Sets Xui Image Color state or configuration.

**Purpose:**

Sets Xui Image Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetMode`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetMode

Sets Xui Image Mode state or configuration.

**Purpose:**

Sets Xui Image Mode state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
```

**Parameters:**

- `pImage`: `xge_xui_image pImage`.
- `iMode`: `int iMode`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageMeasureProc`

---

### xgeXuiImageMeasureProc

Provides the `xgeXuiImageMeasureProc` operation.

**Purpose:**

Provides the `xgeXuiImageMeasureProc` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns a `xge_vec2_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- None.

---

### xgeXuiImagePaintProc

Draws or paints using Xui Image Paint Proc.

**Purpose:**

Draws or paints using Xui Image Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`

---

### xgeXuiInputInit

Initializes the Xui Input object or subsystem.

**Purpose:**

Initializes the Xui Input object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `pContext`: `xge_xui_context pContext`.
- `pWidget`: `xge_xui_widget pWidget`.
- `pFont`: `xge_font pFont`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputUnit

Releases resources associated with Xui Input.

**Purpose:**

Releases resources associated with Xui Input. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputSetText

Sets Xui Input Text state or configuration.

**Purpose:**

Sets Xui Input Text state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputGetText

Gets Xui Input Text state or information.

**Purpose:**

Gets Xui Input Text state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.

**Return Value:**

- Returns a borrowed UTF-8 string pointer. The caller must not free it.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputSetFont

Sets Xui Input Font state or configuration.

**Purpose:**

Sets Xui Input Font state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `pFont`: `xge_font pFont`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputSetColors

Sets Xui Input Colors state or configuration.

**Purpose:**

Sets Xui Input Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `iText`: `uint32_t iText`.
- `iBackground`: `uint32_t iBackground`.
- `iFocus`: `uint32_t iFocus`.
- `iCursor`: `uint32_t iCursor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetSelection`

---

### xgeXuiInputSetSelection

Sets Xui Input Selection state or configuration.

**Purpose:**

Sets Xui Input Selection state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `iStart`: `int iStart`.
- `iEnd`: `int iEnd`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiInputGetSelection

Gets Xui Input Selection state or information.

**Purpose:**

Gets Xui Input Selection state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `pStart`: `int* pStart`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pEnd`: `int* pEnd`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiInputGetCandidateRect

Gets Xui Input Candidate Rect state or information.

**Purpose:**

Gets Xui Input Candidate Rect state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.

**Return Value:**

- Returns a `xge_rect_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.
- Platform backends should prefer `xgeXuiGetImeCandidateRect` for the focused widget; this function remains the Input-specific resolver.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiInputEvent

Processes or dispatches events for Xui Input Event.

**Purpose:**

Processes or dispatches events for Xui Input Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
```

**Parameters:**

- `pInput`: `xge_xui_input pInput`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiInputEventProc

Processes or dispatches events for Xui Input Event Proc.

**Purpose:**

Processes or dispatches events for Xui Input Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiInputPaintProc

Draws or paints using Xui Input Paint Proc.

**Purpose:**

Draws or paints using Xui Input Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`

---

### xgeXuiSliderInit

Initializes the Xui Slider object or subsystem.

**Purpose:**

Initializes the Xui Slider object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `pContext`: `xge_xui_context pContext`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderUnit

Releases resources associated with Xui Slider.

**Purpose:**

Releases resources associated with Xui Slider. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderUnit(xge_xui_slider pSlider);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderSetChange

Sets Xui Slider Change state or configuration.

**Purpose:**

Sets Xui Slider Change state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `procChange`: `xge_xui_slider_proc procChange`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderSetRange

Sets Xui Slider Range state or configuration.

**Purpose:**

Sets Xui Slider Range state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `fMin`: `float fMin`.
- `fMax`: `float fMax`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderSetValue

Sets Xui Slider Value state or configuration.

**Purpose:**

Sets Xui Slider Value state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `fValue`: `float fValue`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderGetValue

Gets Xui Slider Value state or information.

**Purpose:**

Gets Xui Slider Value state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeXuiSliderGetValue(xge_xui_slider pSlider);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.

**Return Value:**

- Returns the requested numeric value. Invalid inputs generally return `0`.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderSetColors`

---

### xgeXuiSliderSetColors

Sets Xui Slider Colors state or configuration.

**Purpose:**

Sets Xui Slider Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `iTrack`: `uint32_t iTrack`.
- `iFill`: `uint32_t iFill`.
- `iKnob`: `uint32_t iKnob`.
- `iFocus`: `uint32_t iFocus`.
- `iDisabled`: `uint32_t iDisabled`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`

---

### xgeXuiSliderGetState

Gets Xui Slider State state or information.

**Purpose:**

Gets Xui Slider State state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiSliderGetState(xge_xui_slider pSlider);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.

**Return Value:**

- Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`

---

### xgeXuiSliderEvent

Processes or dispatches events for Xui Slider Event.

**Purpose:**

Processes or dispatches events for Xui Slider Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent);
```

**Parameters:**

- `pSlider`: `xge_xui_slider pSlider`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`

---

### xgeXuiSliderEventProc

Processes or dispatches events for Xui Slider Event Proc.

**Purpose:**

Processes or dispatches events for Xui Slider Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pEvent`: `const xge_event_t* pEvent`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- Returns an XUI event result or an XGE error code, depending on the API contract.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`

---

### xgeXuiSliderPaintProc

Draws or paints using Xui Slider Paint Proc.

**Purpose:**

Draws or paints using Xui Slider Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`

---

### xgeXuiProgressInit

Initializes the Xui Progress object or subsystem.

**Purpose:**

Initializes the Xui Progress object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressUnit

Releases resources associated with Xui Progress.

**Purpose:**

Releases resources associated with Xui Progress. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressUnit(xge_xui_progress pProgress);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetRange

Sets Xui Progress Range state or configuration.

**Purpose:**

Sets Xui Progress Range state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `fMin`: `float fMin`.
- `fMax`: `float fMax`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetValue

Sets Xui Progress Value state or configuration.

**Purpose:**

Sets Xui Progress Value state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `fValue`: `float fValue`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressGetValue

Gets Xui Progress Value state or information.

**Purpose:**

Gets Xui Progress Value state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeXuiProgressGetValue(xge_xui_progress pProgress);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.

**Return Value:**

- Returns the requested numeric value. Invalid inputs generally return `0`.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetText

Sets Xui Progress Text state or configuration.

**Purpose:**

Sets Xui Progress Text state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetTextColor

Sets Xui Progress Text Color state or configuration.

**Purpose:**

Sets Xui Progress Text Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`

---

### xgeXuiProgressSetColors

Sets Xui Progress Colors state or configuration.

**Purpose:**

Sets Xui Progress Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill);
```

**Parameters:**

- `pProgress`: `xge_xui_progress pProgress`.
- `iTrack`: `uint32_t iTrack`.
- `iFill`: `uint32_t iFill`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`

---

### xgeXuiProgressPaintProc

Draws or paints using Xui Progress Paint Proc.

**Purpose:**

Draws or paints using Xui Progress Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`

---

### xgeXuiPanelInit

Initializes the Xui Panel object or subsystem.

**Purpose:**

Initializes the Xui Panel object, assigns the default widget background/border through the widget base paint path, and installs the title paint callback.

**Prototype:**

```c
XGE_API int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelUnit

Releases resources associated with Xui Panel.

**Purpose:**

Releases resources associated with Xui Panel. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelUnit(xge_xui_panel pPanel);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelSetBackground

Sets Xui Panel Background state or configuration.

**Purpose:**

Sets Xui Panel Background state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelSetTitle

Sets Xui Panel Title state or configuration.

**Purpose:**

Sets Xui Panel Title state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `pFont`: `xge_font pFont`.
- `sTitle`: `const char* sTitle`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelSetTitleColor

Sets Xui Panel Title Color state or configuration.

**Purpose:**

Sets Xui Panel Title Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `iColor`: `uint32_t iColor`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelSetTitleAlign

Sets Xui Panel Title Align state or configuration.

**Purpose:**

Sets Xui Panel Title Align state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `iTextFlags`: `uint32_t iTextFlags`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetClip`

---

### xgeXuiPanelSetClip

Sets Xui Panel Clip state or configuration.

**Purpose:**

Sets Xui Panel Clip state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
```

**Parameters:**

- `pPanel`: `xge_xui_panel pPanel`.
- `bClip`: `int bClip`.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`

---

### xgeXuiPanelPaintProc

Draws or paints using Xui Panel Paint Proc.

**Purpose:**

Draws or paints using Xui Panel Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

**Return Value:**

- This function does not return a value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`

---

### Viewport Rebuild Status

ScrollModel, ScrollFrame, ScrollView, Popup, ListView, TreeView, TableView, TableGrid, TimelineView, PropertyGrid, Menu, ComboBox, TextEdit, ColorPicker, and DatePicker are now implemented around the current viewport architecture. TableGrid common editors including textarea/enum/color/date/time plus picker/file/image entries and XSON `tableGrid` are restored. PropertyGrid is a property-inspector wrapper over the TableGrid/Viewport line. DockPanel/DockLayout is a workbench-level composite control that follows the Widget overlay, capture, focus, XSON, and Window-composition boundary. TableGrid custom editor lifecycle and true immediate mode remain tracked by the dedicated TableGrid spec. The old viewport APIs have been removed.

Authoritative design docs:

- [Viewport / Scroll](../xui/scrollview.md)
- [Viewport Refactor Spec](../xui/viewport-refactor-spec.md)
- [Popup](../xui/popup.md)
- [ListView](../xui/listview.md)
- [TableView](../xui/tableview.md)
- [TableGrid](../xui/tablegrid.md)
- [TreeView](../xui/treeview.md)

Restored XSON types include `scroll` / `scrollView` / `popup` / `listView` / `treeView` / `tableView` / `tableGrid` / `propertyGrid` / `dockLayout`.

The old viewport public API is not kept as a compatibility layer. If a new virtual-data abstraction is needed later, it must be redesigned on top of ScrollModel + ScrollFrame instead of reusing the old base names.

### xgeXuiToastShow

Adds a context-level toast notification. Toast is a convenience service, not a normal widget node and not an XSON page node.

```c
XGE_API int xgeXuiToastShow(xge_xui_context pContext, int iType, const char* sTitle, const char* sMessage, float fDuration, xge_xui_toast_click_proc procClick, void* pUser);
XGE_API int xgeXuiToastClose(xge_xui_context pContext, int iToastId);
XGE_API void xgeXuiToastClear(xge_xui_context pContext);
XGE_API int xgeXuiToastGetActiveCount(xge_xui_context pContext);
XGE_API int xgeXuiToastGetPendingCount(xge_xui_context pContext);
XGE_API void xgeXuiToastSetPlacement(xge_xui_context pContext, int iPlacement);
XGE_API void xgeXuiToastSetDirection(xge_xui_context pContext, int iDirection);
XGE_API void xgeXuiToastSetMetrics(xge_xui_context pContext, float fWidth, float fMargin, float fGap, int iMaxVisible);
XGE_API void xgeXuiToastSetFont(xge_xui_context pContext, xge_font pFont);
XGE_API void xgeXuiToastSetColors(xge_xui_context pContext, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iMutedText, uint32_t iInfo, uint32_t iSuccess, uint32_t iWarning, uint32_t iError);
XGE_API void xgeXuiToastSetClose(xge_xui_context pContext, xge_xui_toast_close_proc procClose, void* pUser);
```

- `fDuration <= 0` uses the default 3 seconds.
- Duration starts when a toast becomes visible, so queued toasts still receive their full visible time.
- Clicking the toast body runs `procClick` when provided and closes with `XGE_XUI_TOAST_CLOSE_CLICK`; the close button closes with `XGE_XUI_TOAST_CLOSE_BUTTON` and does not run the click callback.
- `iMaxVisible == 0` lets the service estimate visible capacity from the root height; the upper bound is `XGE_XUI_TOAST_VISIBLE_CAPACITY`.
- See [Toast](../xui/toast.md) for lifecycle and queue details.

### xgeXuiMsgBoxInit

Binds a MsgBox object to a widget and installs its event and paint callbacks.

```c
XGE_API int xgeXuiMsgBoxInit(xge_xui_msg_box pBox, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiMsgBoxUnit(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetText(xge_xui_msg_box pBox, xge_font pFont, const char* sTitle, const char* sMessage);
XGE_API void xgeXuiMsgBoxSetType(xge_xui_msg_box pBox, int iType);
XGE_API void xgeXuiMsgBoxSetIconTexture(xge_xui_msg_box pBox, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiMsgBoxSetButtons(xge_xui_msg_box pBox, int iButtons);
XGE_API void xgeXuiMsgBoxSetCustomButtons(xge_xui_msg_box pBox, xvalue arrButtons);
XGE_API void xgeXuiMsgBoxSetResult(xge_xui_msg_box pBox, xge_xui_select_proc procResult, void* pUser);
XGE_API void xgeXuiMsgBoxSetModal(xge_xui_msg_box pBox, int bModal);
XGE_API void xgeXuiMsgBoxSetOpen(xge_xui_msg_box pBox, int bOpen);
XGE_API int xgeXuiMsgBoxIsOpen(xge_xui_msg_box pBox);
XGE_API int xgeXuiMsgBoxGetResult(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetColors(xge_xui_msg_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iMessage, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiMsgBoxEvent(xge_xui_msg_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiMsgBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiMsgBoxPaintProc(xge_xui_widget pWidget, void* pUser);
```

### xgeXuiInputBoxInit

Binds an InputBox object to a widget and uses the same window-based overlay behavior as MsgBox.

```c
XGE_API int xgeXuiInputBoxInit(xge_xui_input_box pBox, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
XGE_API void xgeXuiInputBoxUnit(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetText(xge_xui_input_box pBox, xge_font pFont, const char* sTitle, const char* sPrompt, const char* sInitial);
XGE_API void xgeXuiInputBoxSetResult(xge_xui_input_box pBox, xge_xui_text_submit_proc procResult, void* pUser);
XGE_API void xgeXuiInputBoxSetModal(xge_xui_input_box pBox, int bModal);
XGE_API void xgeXuiInputBoxSetOpen(xge_xui_input_box pBox, int bOpen);
XGE_API int xgeXuiInputBoxIsOpen(xge_xui_input_box pBox);
XGE_API int xgeXuiInputBoxGetResultCode(xge_xui_input_box pBox);
XGE_API char* xgeXuiInputBoxGetResult(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetColors(xge_xui_input_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iPrompt, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiInputBoxEvent(xge_xui_input_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiInputBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputBoxPaintProc(xge_xui_widget pWidget, void* pUser);
```

## Lifecycle And Ownership

Resource APIs use explicit lifecycle management. Pair initialization with unit/free calls, pair load/create/open calls with the matching release calls, and keep borrowed pointers valid while the engine may read them.

## Threading

Follow the module-level thread model. Rendering and GPU-backed resources may be submitted from worker-side queues but are ultimately executed on the graphics context owner thread.

## Backend Differences

Desktop GL, OpenGL ES, WebGL2, mini program hosts, EGL offscreen contexts, and board Linux backends may expose different limits. Query capabilities where available and keep fallback paths for unsupported features.

## Related Documents

- [Architecture](../ARCHITECTURE.en.md)
- [Compatibility](../COMPATIBILITY.en.md)
- [Guides](../guide/README.en.md)
- [Cases](../case/README.en.md)
