# Render Target

> Window and offscreen render targets, render passes, GPU buffers, readback, and manual refresh workflows.

[Back to API Reference](README.en.md) | [Chinese Source](render-target.md)

---

## Module Role

Window and offscreen render targets, render passes, GPU buffers, readback, and manual refresh workflows.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeRenderTargetWindow

Gets Render Target Window state or information.

**Purpose:**

Gets Render Target Window state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeRenderTargetWindow(xge_render_target pTarget);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.

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

### xgeRenderTargetCreate

Creates a Render Target resource.

**Purpose:**

Creates a Render Target resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.
- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.

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

- `xgeRenderTargetWindow`
- `xgeRenderTargetResize`
- `xgeRenderTargetReadPixels`
- `xgeRenderTargetTexture`
- `xgeRenderTargetFree`

---

### xgeRenderTargetResize

Gets Render Target Resize state or information.

**Purpose:**

Gets Render Target Resize state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.
- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.

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

### xgeRenderTargetReadPixels

Gets Render Target Read Pixels state or information.

**Purpose:**

Gets Render Target Read Pixels state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.
- `pPixels`: `void* pPixels`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iStride`: `int iStride`.

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

### xgeRenderTargetTexture

Gets Render Target Texture state or information.

**Purpose:**

Gets Render Target Texture state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_texture xgeRenderTargetTexture(xge_render_target pTarget);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.

**Return Value:**

- Returns a `xge_texture` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

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

### xgeRenderTargetFree

Releases resources associated with Render Target.

**Purpose:**

Releases resources associated with Render Target. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeRenderTargetFree(xge_render_target pTarget);
```

**Parameters:**

- `pTarget`: `xge_render_target pTarget`.

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

- `xgeRenderTargetWindow`
- `xgeRenderTargetCreate`
- `xgeRenderTargetResize`
- `xgeRenderTargetReadPixels`
- `xgeRenderTargetTexture`

---

### xgeBufferCreate

Creates a Buffer resource.

**Purpose:**

Creates a Buffer resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeBufferCreate(xge_buffer pBuffer, int iType, int iUsage, const void* pData, int iSize);
```

**Parameters:**

- `pBuffer`: `xge_buffer pBuffer`.
- `iType`: `int iType`.
- `iUsage`: `int iUsage`.
- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.

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

- `xgeBufferUpdate`
- `xgeBufferUpload`
- `xgeBufferFree`

---

### xgeBufferUpdate

Updates Buffer Update state.

**Purpose:**

Updates Buffer Update state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeBufferUpdate(xge_buffer pBuffer, int iOffset, const void* pData, int iSize);
```

**Parameters:**

- `pBuffer`: `xge_buffer pBuffer`.
- `iOffset`: `int iOffset`.
- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.

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

### xgeBufferUpload

Loads or opens the Buffer Upload resource.

**Purpose:**

Loads or opens the Buffer Upload resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeBufferUpload(xge_buffer pBuffer);
```

**Parameters:**

- `pBuffer`: `xge_buffer pBuffer`.

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

### xgeBufferFree

Releases resources associated with Buffer.

**Purpose:**

Releases resources associated with Buffer. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeBufferFree(xge_buffer pBuffer);
```

**Parameters:**

- `pBuffer`: `xge_buffer pBuffer`.

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

- `xgeBufferCreate`
- `xgeBufferUpdate`
- `xgeBufferUpload`

---

### xgePassInit

Initializes the Pass object or subsystem.

**Purpose:**

Initializes the Pass object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgePassInit(xge_pass pPass, xge_render_target pTarget, uint32_t iClearFlags, uint32_t iClearColor);
```

**Parameters:**

- `pPass`: `xge_pass pPass`.
- `pTarget`: `xge_render_target pTarget`.
- `iClearFlags`: `uint32_t iClearFlags`.
- `iClearColor`: `uint32_t iClearColor`.

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

- `xgePassBegin`
- `xgePassEnd`

---

### xgePassBegin

Begins the Pass Begin operation.

**Purpose:**

Begins the Pass Begin operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgePassBegin(xge_pass pPass);
```

**Parameters:**

- `pPass`: `xge_pass pPass`.

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

### xgePassEnd

Ends the Pass End operation.

**Purpose:**

Ends the Pass End operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgePassEnd(xge_pass pPass);
```

**Parameters:**

- `pPass`: `xge_pass pPass`.

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
