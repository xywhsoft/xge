# Platform

> Platform and graphics backend selection, runtime capabilities, GPU caps, and shader/profile mapping.

[Back to API Reference](README.en.md) | [Chinese Source](platform.md)

---

## Module Role

Platform and graphics backend selection, runtime capabilities, GPU caps, and shader/profile mapping.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgePlatformBackendDefault

Ends the Platform Backend Default operation.

**Purpose:**

Ends the Platform Backend Default operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_platform_backend_t xgePlatformBackendDefault(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_platform_backend_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

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

### xgePlatformBackendSet

Sets Platform Backend Set state or configuration.

**Purpose:**

Sets Platform Backend Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgePlatformBackendSet(const xge_platform_backend_t* pBackend);
```

**Parameters:**

- `pBackend`: `const xge_platform_backend_t* pBackend`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgePlatformBackendDefault`
- `xgePlatformBackendGet`

---

### xgePlatformBackendGet

Gets Platform Backend Get state or information.

**Purpose:**

Gets Platform Backend Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_platform_backend_t xgePlatformBackendGet(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_platform_backend_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgePlatformBackendDefault`
- `xgePlatformBackendSet`

---

### xgeGraphicsBackendDefault

Ends the Graphics Backend Default operation.

**Purpose:**

Ends the Graphics Backend Default operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_graphics_backend_t xgeGraphicsBackendDefault(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_graphics_backend_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

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

### xgeGraphicsBackendSet

Sets Graphics Backend Set state or configuration.

**Purpose:**

Sets Graphics Backend Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeGraphicsBackendSet(const xge_graphics_backend_t* pBackend);
```

**Parameters:**

- `pBackend`: `const xge_graphics_backend_t* pBackend`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendGet`

---

### xgeGraphicsBackendGet

Gets Graphics Backend Get state or information.

**Purpose:**

Gets Graphics Backend Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_graphics_backend_t xgeGraphicsBackendGet(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_graphics_backend_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendSet`

---

### xgeGpuCapsGet

Gets Gpu Caps Get state or information.

**Purpose:**

Gets Gpu Caps Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeGpuCapsGet(xge_gpu_caps_t* pCaps);
```

**Parameters:**

- `pCaps`: `xge_gpu_caps_t* pCaps`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgePlatformCapsGet

Gets Platform Caps Get state or information.

**Purpose:**

Gets Platform Caps Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgePlatformCapsGet(xge_platform_caps_t* pCaps);
```

**Parameters:**

- `pCaps`: `xge_platform_caps_t* pCaps`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgePlatformRuntimeGet

Gets Platform Runtime Get state or information.

**Purpose:**

Gets Platform Runtime Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgePlatformRuntimeGet(xge_platform_runtime_t* pRuntime);
```

**Parameters:**

- `pRuntime`: `xge_platform_runtime_t* pRuntime`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeGraphicsShaderHeaderGet

Gets Graphics Shader Header Get state or information.

**Purpose:**

Gets Graphics Shader Header Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize);
```

**Parameters:**

- `iBackend`: `int iBackend`.
- `sBuffer`: `char* sBuffer`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.

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

### xgeGraphicsLibraryNameGet

Gets Graphics Library Name Get state or information.

**Purpose:**

Gets Graphics Library Name Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeGraphicsLibraryNameGet(int iBackend, int iIndex, char* sBuffer, int iSize);
```

**Parameters:**

- `iBackend`: `int iBackend`.
- `iIndex`: `int iIndex`.
- `sBuffer`: `char* sBuffer`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.

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

### xgeGraphicsMappingGet

Gets Graphics Mapping Get state or information.

**Purpose:**

Gets Graphics Mapping Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeGraphicsMappingGet(int iBackend, xge_graphics_mapping_t* pMapping);
```

**Parameters:**

- `iBackend`: `int iBackend`.
- `pMapping`: `xge_graphics_mapping_t* pMapping`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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
