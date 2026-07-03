# Offscreen

> EGL initialization, offscreen contexts, pbuffer/surfaceless usage, render targets, and pixel readback.

[Back to API Reference](README.en.md) | [Chinese Source](offscreen.md)

---

## Module Role

EGL initialization, offscreen contexts, pbuffer/surfaceless usage, render targets, and pixel readback.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeEGLCapsGet

Gets EGLCaps Get state or information.

**Purpose:**

Gets EGLCaps Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeEGLCapsGet(xge_egl_caps_t* pCaps);
```

**Parameters:**

- `pCaps`: `xge_egl_caps_t* pCaps`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeEGLInit

Initializes the EGLInit object or subsystem.

**Purpose:**

Initializes the EGLInit object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc);
```

**Parameters:**

- `pContext`: `xge_egl_context_t* pContext`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pDesc`: `const xge_egl_desc_t* pDesc`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeEGLCapsGet`
- `xgeEGLUnit`
- `xgeEGLMakeCurrent`

---

### xgeEGLUnit

Releases resources associated with EGLUnit.

**Purpose:**

Releases resources associated with EGLUnit. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeEGLUnit(xge_egl_context_t* pContext);
```

**Parameters:**

- `pContext`: `xge_egl_context_t* pContext`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeEGLCapsGet`
- `xgeEGLInit`
- `xgeEGLMakeCurrent`

---

### xgeEGLMakeCurrent

Provides the `xgeEGLMakeCurrent` operation.

**Purpose:**

Provides the `xgeEGLMakeCurrent` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeEGLMakeCurrent(xge_egl_context_t* pContext);
```

**Parameters:**

- `pContext`: `xge_egl_context_t* pContext`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeOffscreenInit

Initializes the Offscreen object or subsystem.

**Purpose:**

Initializes the Offscreen object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight);
```

**Parameters:**

- `pOffscreen`: `xge_offscreen pOffscreen`.
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

- `xgeOffscreenUnit`
- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`

---

### xgeOffscreenUnit

Releases resources associated with Offscreen.

**Purpose:**

Releases resources associated with Offscreen. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeOffscreenUnit(xge_offscreen pOffscreen);
```

**Parameters:**

- `pOffscreen`: `xge_offscreen pOffscreen`.

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

- `xgeOffscreenInit`
- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`

---

### xgeOffscreenRenderTarget

Gets Offscreen Render Target state or information.

**Purpose:**

Gets Offscreen Render Target state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen);
```

**Parameters:**

- `pOffscreen`: `xge_offscreen pOffscreen`.

**Return Value:**

- Returns a `xge_render_target` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

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

### xgeOffscreenReadPixels

Provides the `xgeOffscreenReadPixels` operation.

**Purpose:**

Provides the `xgeOffscreenReadPixels` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride);
```

**Parameters:**

- `pOffscreen`: `xge_offscreen pOffscreen`.
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
