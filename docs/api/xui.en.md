# XUI

> XUI incubation APIs: retained widgets, layout, host bridge, event dispatch, text editing, and standard controls.

[Back to API Reference](README.en.md) | [Chinese Source](xui.md)

---

## Module Role

XUI incubation APIs: retained widgets, layout, host bridge, event dispatch, text editing, and standard controls.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

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

Gets Xui Widget Add state or information.

**Purpose:**

Gets Xui Widget Add state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
```

**Parameters:**

- `pParent`: `xge_xui_widget pParent`.
- `pChild`: `xge_xui_widget pChild`.

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

Gets Xui Widget Rect state or information.

**Purpose:**

Gets Xui Widget Rect state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
```

**Parameters:**

- `pWidget`: `xge_xui_widget pWidget`.

**Return Value:**

- Returns a `xge_rect_t` value.

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
- `fRowHeight`: `float fRowHeight`.
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

Sets Xui Widget Z state or configuration.

**Purpose:**

Sets Xui Widget Z state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

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

### xgeXuiWidgetGetZ

Gets Xui Widget Z state or information.

**Purpose:**

Gets Xui Widget Z state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
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

### xgeXuiWidgetGetFlags

Gets Xui Widget Flags state or information.

**Purpose:**

Gets Xui Widget Flags state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

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

### xgeXuiWidgetSetClip

Sets Xui Widget Clip state or configuration.

**Purpose:**

Sets Xui Widget Clip state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

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

Sets Xui Capture state or configuration.

**Purpose:**

Sets Xui Capture state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

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

### xgeXuiToggleInit

Initializes the Xui Toggle object or subsystem.

**Purpose:**

Initializes the Xui Toggle object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
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

- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleUnit

Releases resources associated with Xui Toggle.

**Purpose:**

Releases resources associated with Xui Toggle. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleUnit(xge_xui_toggle pToggle);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.

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

- `xgeXuiToggleInit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleSetChange

Sets Xui Toggle Change state or configuration.

**Purpose:**

Sets Xui Toggle Change state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_toggle_proc procChange, void* pUser);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
- `procChange`: `xge_xui_toggle_proc procChange`.
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleSetText

Sets Xui Toggle Text state or configuration.

**Purpose:**

Sets Xui Toggle Text state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleSetText(xge_xui_toggle pToggle, xge_font pFont, const char* sText);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleSetChecked

Sets Xui Toggle Checked state or configuration.

**Purpose:**

Sets Xui Toggle Checked state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
- `bChecked`: `int bChecked`.

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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleGetChecked

Gets Xui Toggle Checked state or information.

**Purpose:**

Gets Xui Toggle Checked state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiToggleGetChecked(xge_xui_toggle pToggle);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.

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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleSetTextColor

Sets Xui Toggle Text Color state or configuration.

**Purpose:**

Sets Xui Toggle Text Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleSetTextColor(xge_xui_toggle pToggle, uint32_t iColor);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleSetColors

Sets Xui Toggle Colors state or configuration.

**Purpose:**

Sets Xui Toggle Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iChecked);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
- `iNormal`: `uint32_t iNormal`.
- `iHover`: `uint32_t iHover`.
- `iActive`: `uint32_t iActive`.
- `iFocus`: `uint32_t iFocus`.
- `iDisabled`: `uint32_t iDisabled`.
- `iChecked`: `uint32_t iChecked`.

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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleGetState

Gets Xui Toggle State state or information.

**Purpose:**

Gets Xui Toggle State state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiToggleGetState(xge_xui_toggle pToggle);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.

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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleEvent

Processes or dispatches events for Xui Toggle Event.

**Purpose:**

Processes or dispatches events for Xui Toggle Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent);
```

**Parameters:**

- `pToggle`: `xge_xui_toggle pToggle`.
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleEventProc

Processes or dispatches events for Xui Toggle Event Proc.

**Purpose:**

Processes or dispatches events for Xui Toggle Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

---

### xgeXuiTogglePaintProc

Draws or paints using Xui Toggle Paint Proc.

**Purpose:**

Draws or paints using Xui Toggle Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser);
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

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`

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

Initializes the Xui Panel object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

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

### xgeXuiScrollViewInit

Initializes the Xui Scroll View object or subsystem.

**Purpose:**

Initializes the Xui Scroll View object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
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

- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewUnit

Releases resources associated with Xui Scroll View.

**Purpose:**

Releases resources associated with Xui Scroll View. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.

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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewSetContentSize

Sets Xui Scroll View Content Size state or configuration.

**Purpose:**

Sets Xui Scroll View Content Size state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
- `fWidth`: `float fWidth`.
- `fHeight`: `float fHeight`.

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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewSetOffset

Sets Xui Scroll View Offset state or configuration.

**Purpose:**

Sets Xui Scroll View Offset state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
- `fX`: `float fX`.
- `fY`: `float fY`.

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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewGetOffset

Sets Xui Scroll View Get Offset state or configuration.

**Purpose:**

Sets Xui Scroll View Get Offset state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
- `pX`: `float* pX`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pY`: `float* pY`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewSetColors

Sets Xui Scroll View Colors state or configuration.

**Purpose:**

Sets Xui Scroll View Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
- `iBackground`: `uint32_t iBackground`.
- `iBar`: `uint32_t iBar`.
- `iThumb`: `uint32_t iThumb`.

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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewEvent`

---

### xgeXuiScrollViewEvent

Processes or dispatches events for Xui Scroll View Event.

**Purpose:**

Processes or dispatches events for Xui Scroll View Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent);
```

**Parameters:**

- `pScroll`: `xge_xui_scroll_view pScroll`.
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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`

---

### xgeXuiScrollViewEventProc

Processes or dispatches events for Xui Scroll View Event Proc.

**Purpose:**

Processes or dispatches events for Xui Scroll View Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`

---

### xgeXuiScrollViewPaintProc

Draws or paints using Xui Scroll View Paint Proc.

**Purpose:**

Draws or paints using Xui Scroll View Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiScrollViewPaintProc(xge_xui_widget pWidget, void* pUser);
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

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`

---

### xgeXuiListViewInit

Initializes the Xui List View object or subsystem.

**Purpose:**

Initializes the Xui List View object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
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

- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewUnit

Releases resources associated with Xui List View.

**Purpose:**

Releases resources associated with Xui List View. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewUnit(xge_xui_list_view pList);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewSetItems

Sets Xui List View Items state or configuration.

**Purpose:**

Sets Xui List View Items state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `arrItems`: `const char** arrItems`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iCount`: `int iCount`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewSetFont

Sets Xui List View Font state or configuration.

**Purpose:**

Sets Xui List View Font state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewSetItemHeight

Sets Xui List View Item Height state or configuration.

**Purpose:**

Sets Xui List View Item Height state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `fHeight`: `float fHeight`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewSetSelected

Sets Xui List View Selected state or configuration.

**Purpose:**

Sets Xui List View Selected state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `iIndex`: `int iIndex`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewGetSelected

Gets Xui List View Selected state or information.

**Purpose:**

Gets Xui List View Selected state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiListViewGetSelected(xge_xui_list_view pList);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewSetScroll

Sets Xui List View Scroll state or configuration.

**Purpose:**

Sets Xui List View Scroll state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `fScrollY`: `float fScrollY`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewGetScroll

Gets Xui List View Scroll state or information.

**Purpose:**

Gets Xui List View Scroll state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeXuiListViewGetScroll(xge_xui_list_view pList);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewSetSelect

Sets Xui List View Select state or configuration.

**Purpose:**

Sets Xui List View Select state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `procSelect`: `xge_xui_select_proc procSelect`.
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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewSetColors

Sets Xui List View Colors state or configuration.

**Purpose:**

Sets Xui List View Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
- `iBackground`: `uint32_t iBackground`.
- `iRow`: `uint32_t iRow`.
- `iSelected`: `uint32_t iSelected`.
- `iText`: `uint32_t iText`.
- `iBar`: `uint32_t iBar`.
- `iThumb`: `uint32_t iThumb`.

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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewEvent

Processes or dispatches events for Xui List View Event.

**Purpose:**

Processes or dispatches events for Xui List View Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent);
```

**Parameters:**

- `pList`: `xge_xui_list_view pList`.
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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewEventProc

Processes or dispatches events for Xui List View Event Proc.

**Purpose:**

Processes or dispatches events for Xui List View Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewPaintProc

Draws or paints using Xui List View Paint Proc.

**Purpose:**

Draws or paints using Xui List View Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser);
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

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`

---

### xgeXuiDialogInit

Initializes the Xui Dialog object or subsystem.

**Purpose:**

Initializes the Xui Dialog object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDialogInit(xge_xui_dialog pDialog, xge_xui_context pContext, xge_xui_widget pWidget);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
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

- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetColors`

---

### xgeXuiDialogUnit

Releases resources associated with Xui Dialog.

**Purpose:**

Releases resources associated with Xui Dialog. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogUnit(xge_xui_dialog pDialog);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.

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

- `xgeXuiDialogInit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetColors`

---

### xgeXuiDialogSetTitle

Sets Xui Dialog Title state or configuration.

**Purpose:**

Sets Xui Dialog Title state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogSetTitle(xge_xui_dialog pDialog, xge_font pFont, const char* sTitle);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetColors`

---

### xgeXuiDialogSetClose

Releases resources associated with Xui Dialog Set.

**Purpose:**

Releases resources associated with Xui Dialog Set. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogSetClose(xge_xui_dialog pDialog, xge_xui_click_proc procClose, void* pUser);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
- `procClose`: `xge_xui_click_proc procClose`.
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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetColors`

---

### xgeXuiDialogSetOpen

Loads or opens the Xui Dialog Set resource.

**Purpose:**

Loads or opens the Xui Dialog Set resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogSetOpen(xge_xui_dialog pDialog, int bOpen);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
- `bOpen`: `int bOpen`.

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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetColors`

---

### xgeXuiDialogIsOpen

Loads or opens the Xui Dialog Is resource.

**Purpose:**

Loads or opens the Xui Dialog Is resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDialogIsOpen(xge_xui_dialog pDialog);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.

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

### xgeXuiDialogSetColors

Sets Xui Dialog Colors state or configuration.

**Purpose:**

Sets Xui Dialog Colors state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogSetColors(xge_xui_dialog pDialog, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
- `iBackdrop`: `uint32_t iBackdrop`.
- `iBackground`: `uint32_t iBackground`.
- `iTitle`: `uint32_t iTitle`.
- `iClose`: `uint32_t iClose`.

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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`

---

### xgeXuiDialogEvent

Processes or dispatches events for Xui Dialog Event.

**Purpose:**

Processes or dispatches events for Xui Dialog Event. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDialogEvent(xge_xui_dialog pDialog, const xge_event_t* pEvent);
```

**Parameters:**

- `pDialog`: `xge_xui_dialog pDialog`.
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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`

---

### xgeXuiDialogEventProc

Processes or dispatches events for Xui Dialog Event Proc.

**Purpose:**

Processes or dispatches events for Xui Dialog Event Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeXuiDialogEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`

---

### xgeXuiDialogPaintProc

Draws or paints using Xui Dialog Paint Proc.

**Purpose:**

Draws or paints using Xui Dialog Paint Proc. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeXuiDialogPaintProc(xge_xui_widget pWidget, void* pUser);
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

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`

---

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
