# Async

> Asynchronous loading, request polling, cancellation, fallback assets, and GPU upload coordination.

[Back to API Reference](README.en.md) | [Chinese Source](async.md)

---

## Module Role

Asynchronous loading, request polling, cancellation, fallback assets, and GPU upload coordination.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeAsyncRequestInit

Initializes the Async Request object or subsystem.

**Purpose:**

Initializes the Async Request object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAsyncRequestInit(xge_async_request pRequest);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.

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

- `xgeAsyncRequestFree`
- `xgeAsyncRequestCancel`

---

### xgeAsyncRequestFree

Releases resources associated with Async Request.

**Purpose:**

Releases resources associated with Async Request. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAsyncRequestFree(xge_async_request pRequest);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.

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

- `xgeAsyncRequestInit`
- `xgeAsyncRequestCancel`

---

### xgeAsyncRequestCancel

Provides the `xgeAsyncRequestCancel` operation.

**Purpose:**

Provides the `xgeAsyncRequestCancel` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncRequestCancel(xge_async_request pRequest);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.

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

### xgeAsyncPoll

Provides the `xgeAsyncPoll` operation.

**Purpose:**

Provides the `xgeAsyncPoll` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncPoll(xge_async_request pRequest);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.

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

### xgeAsyncThreadingSet

Sets Async Threading Set state or configuration.

**Purpose:**

Sets Async Threading Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncThreadingSet(int bEnabled);
```

**Parameters:**

- `bEnabled`: `int bEnabled`.

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

- `xgeAsyncThreadingGet`

---

### xgeAsyncThreadingGet

Gets Async Threading Get state or information.

**Purpose:**

Gets Async Threading Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncThreadingGet(void);
```

**Parameters:**

None.

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

- `xgeAsyncThreadingSet`

---

### xgeAsyncImageLoad

Loads or opens the Async Image resource.

**Purpose:**

Loads or opens the Async Image resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.
- `pImage`: `xge_image pImage`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iFlags`: `uint32_t iFlags`.
- `onComplete`: `xge_async_proc onComplete`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeAsyncTextureLoad

Loads or opens the Async Texture resource.

**Purpose:**

Loads or opens the Async Texture resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.
- `pTexture`: `xge_texture pTexture`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iFlags`: `uint32_t iFlags`.
- `onComplete`: `xge_async_proc onComplete`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeAsyncFontLoad

Loads or opens the Async Font resource.

**Purpose:**

Loads or opens the Async Font resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.
- `pFont`: `xge_font pFont`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fSize`: `float fSize`.
- `onComplete`: `xge_async_proc onComplete`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeAsyncSoundLoad

Loads or opens the Async Sound resource.

**Purpose:**

Loads or opens the Async Sound resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser);
```

**Parameters:**

- `pRequest`: `xge_async_request pRequest`.
- `pSound`: `xge_sound pSound`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `onComplete`: `xge_async_proc onComplete`.
- `pUser`: `void* pUser`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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
