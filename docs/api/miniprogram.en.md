# Miniprogram

> Mini program host integration, frame driving, resize, touch, text, and audio bridge commands.

[Back to API Reference](README.en.md) | [Chinese Source](miniprogram.md)

---

## Module Role

Mini program host integration, frame driving, resize, touch, text, and audio bridge commands.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeMiniProgramInit

Initializes the Mini Program object or subsystem.

**Purpose:**

Initializes the Mini Program object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc);
```

**Parameters:**

- `pDesc`: `const xge_miniprogram_desc_t* pDesc`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeMiniProgramInitSimple`
- `xgeMiniProgramUnit`
- `xgeMiniProgramSetBridge`
- `xgeMiniProgramFrame`
- `xgeMiniProgramResize`
- `xgeMiniProgramTouch`

---

### xgeMiniProgramInitSimple

Initializes the Mini Program Simple object or subsystem.

**Purpose:**

Initializes the Mini Program Simple object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio);
```

**Parameters:**

- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.
- `fDevicePixelRatio`: `float fDevicePixelRatio`.

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

- `xgeMiniProgramInit`
- `xgeMiniProgramUnit`
- `xgeMiniProgramSetBridge`
- `xgeMiniProgramFrame`
- `xgeMiniProgramResize`
- `xgeMiniProgramTouch`

---

### xgeMiniProgramUnit

Releases resources associated with Mini Program.

**Purpose:**

Releases resources associated with Mini Program. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMiniProgramUnit(void);
```

**Parameters:**

None.

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

- `xgeMiniProgramInit`
- `xgeMiniProgramInitSimple`
- `xgeMiniProgramSetBridge`
- `xgeMiniProgramFrame`
- `xgeMiniProgramResize`
- `xgeMiniProgramTouch`

---

### xgeMiniProgramSetBridge

Sets Mini Program Bridge state or configuration.

**Purpose:**

Sets Mini Program Bridge state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge);
```

**Parameters:**

- `pBridge`: `const xge_miniprogram_bridge_t* pBridge`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeMiniProgramInit`
- `xgeMiniProgramInitSimple`
- `xgeMiniProgramUnit`
- `xgeMiniProgramFrame`
- `xgeMiniProgramResize`
- `xgeMiniProgramTouch`

---

### xgeMiniProgramFrame

Provides the `xgeMiniProgramFrame` operation.

**Purpose:**

Provides the `xgeMiniProgramFrame` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramFrame(double fTimeSeconds);
```

**Parameters:**

- `fTimeSeconds`: `double fTimeSeconds`.

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

### xgeMiniProgramResize

Provides the `xgeMiniProgramResize` operation.

**Purpose:**

Provides the `xgeMiniProgramResize` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio);
```

**Parameters:**

- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.
- `fDevicePixelRatio`: `float fDevicePixelRatio`.

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

### xgeMiniProgramTouch

Provides the `xgeMiniProgramTouch` operation.

**Purpose:**

Provides the `xgeMiniProgramTouch` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount);
```

**Parameters:**

- `iPhase`: `int iPhase`.
- `pTouches`: `const xge_miniprogram_touch_t* pTouches`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iCount`: `int iCount`.

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

- `xgeMiniProgramTouchOne`

---

### xgeMiniProgramTouchOne

Provides the `xgeMiniProgramTouchOne` operation.

**Purpose:**

Provides the `xgeMiniProgramTouchOne` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce);
```

**Parameters:**

- `iPhase`: `int iPhase`.
- `iId`: `int iId`.
- `fX`: `float fX`.
- `fY`: `float fY`.
- `fForce`: `float fForce`.

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

### xgeMiniProgramText

Provides the `xgeMiniProgramText` operation.

**Purpose:**

Provides the `xgeMiniProgramText` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramText(uint32_t iCodepoint);
```

**Parameters:**

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

### xgeMiniProgramRequestFrame

Provides the `xgeMiniProgramRequestFrame` operation.

**Purpose:**

Provides the `xgeMiniProgramRequestFrame` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramRequestFrame(void);
```

**Parameters:**

None.

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

### xgeMiniProgramAudioCommand

Provides the `xgeMiniProgramAudioCommand` operation.

**Purpose:**

Provides the `xgeMiniProgramAudioCommand` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize);
```

**Parameters:**

- `iCommand`: `int iCommand`.
- `iHandle`: `int iHandle`.
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
