# Resource

> Resource providers, file/memory loading, res:// paths, optional xpack providers, and resource ownership.

[Back to API Reference](README.en.md) | [Chinese Source](resource.md)

---

## Module Role

Resource providers, file/memory loading, res:// paths, optional xpack providers, and resource ownership.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeResourceProviderAdd

Provides the `xgeResourceProviderAdd` operation.

**Purpose:**

Provides the `xgeResourceProviderAdd` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);
```

**Parameters:**

- `pProvider`: `const xge_resource_provider_t* pProvider`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeResourceProviderClear

Clears or resets Resource Provider Clear state.

**Purpose:**

Clears or resets Resource Provider Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeResourceProviderClear(void);
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

- None.

---

### xgeResourceXPackProviderAdd

Provides the `xgeResourceXPackProviderAdd` operation.

**Purpose:**

Provides the `xgeResourceXPackProviderAdd` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* pProvider);
```

**Parameters:**

- `pProvider`: `const xge_xpack_provider_t* pProvider`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeResourceLoad

Loads or opens the Resource resource.

**Purpose:**

Loads or opens the Resource resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);
```

**Parameters:**

- `sURI`: `const char* sURI`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pResource`: `xge_resource_t* pResource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoadMemory`
- `xgeResourceFree`

---

### xgeResourceLoadMemory

Loads or opens the Resource Memory resource.

**Purpose:**

Loads or opens the Resource Memory resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);
```

**Parameters:**

- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.
- `pResource`: `xge_resource_t* pResource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoad`
- `xgeResourceFree`

---

### xgeResourceFree

Releases resources associated with Resource.

**Purpose:**

Releases resources associated with Resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeResourceFree(xge_resource_t* pResource);
```

**Parameters:**

- `pResource`: `xge_resource_t* pResource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoad`
- `xgeResourceLoadMemory`

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
