# Material

> Shaders, uniforms, shader variants, materials, buffers, meshes, and custom rendering entry points.

[Back to API Reference](README.en.md) | [Chinese Source](material.md)

---

## Module Role

Shaders, uniforms, shader variants, materials, buffers, meshes, and custom rendering entry points.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeShaderCreate

Creates a Shader resource.

**Purpose:**

Creates a Shader resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderCreate(xge_shader pShader, const char* sVertexSource, const char* sFragmentSource);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.
- `sVertexSource`: `const char* sVertexSource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `sFragmentSource`: `const char* sFragmentSource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeShaderAddRef`
- `xgeShaderFree`
- `xgeShaderUniform1f`
- `xgeShaderUniform2f`
- `xgeShaderUniform3f`
- `xgeShaderUniform4f`

---

### xgeShaderAddRef

Provides the `xgeShaderAddRef` operation.

**Purpose:**

Provides the `xgeShaderAddRef` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderAddRef(xge_shader pShader);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.

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

### xgeShaderFree

Releases resources associated with Shader.

**Purpose:**

Releases resources associated with Shader. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShaderFree(xge_shader pShader);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.

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

- `xgeShaderCreate`
- `xgeShaderAddRef`
- `xgeShaderUniform1f`
- `xgeShaderUniform2f`
- `xgeShaderUniform3f`
- `xgeShaderUniform4f`

---

### xgeShaderUniform1f

Provides the `xgeShaderUniform1f` operation.

**Purpose:**

Provides the `xgeShaderUniform1f` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderUniform1f(xge_shader pShader, const char* sName, float fX);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fX`: `float fX`.

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

### xgeShaderUniform2f

Provides the `xgeShaderUniform2f` operation.

**Purpose:**

Provides the `xgeShaderUniform2f` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderUniform2f(xge_shader pShader, const char* sName, float fX, float fY);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fX`: `float fX`.
- `fY`: `float fY`.

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

### xgeShaderUniform3f

Provides the `xgeShaderUniform3f` operation.

**Purpose:**

Provides the `xgeShaderUniform3f` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderUniform3f(xge_shader pShader, const char* sName, float fX, float fY, float fZ);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fX`: `float fX`.
- `fY`: `float fY`.
- `fZ`: `float fZ`.

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

### xgeShaderUniform4f

Provides the `xgeShaderUniform4f` operation.

**Purpose:**

Provides the `xgeShaderUniform4f` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderUniform4f(xge_shader pShader, const char* sName, float fX, float fY, float fZ, float fW);
```

**Parameters:**

- `pShader`: `xge_shader pShader`.
- `sName`: `const char* sName`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fX`: `float fX`.
- `fY`: `float fY`.
- `fZ`: `float fZ`.
- `fW`: `float fW`.

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

### xgeShaderVariantSetInit

Initializes the Shader Variant Set object or subsystem.

**Purpose:**

Initializes the Shader Variant Set object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderVariantSetInit(xge_shader_variant_set pSet, const char* sVertexSource, const char* sFragmentSource);
```

**Parameters:**

- `pSet`: `xge_shader_variant_set pSet`.
- `sVertexSource`: `const char* sVertexSource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `sFragmentSource`: `const char* sFragmentSource`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeShaderVariantSetFree`
- `xgeShaderVariantGet`

---

### xgeShaderVariantSetFree

Releases resources associated with Shader Variant Set.

**Purpose:**

Releases resources associated with Shader Variant Set. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShaderVariantSetFree(xge_shader_variant_set pSet);
```

**Parameters:**

- `pSet`: `xge_shader_variant_set pSet`.

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

- `xgeShaderVariantSetInit`
- `xgeShaderVariantGet`

---

### xgeShaderVariantGet

Gets Shader Variant Get state or information.

**Purpose:**

Gets Shader Variant Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShaderVariantGet(xge_shader_variant_set pSet, uint32_t iKey, const xge_shader_define_t* pDefines, int iDefineCount, xge_shader* ppShader);
```

**Parameters:**

- `pSet`: `xge_shader_variant_set pSet`.
- `iKey`: `uint32_t iKey`.
- `pDefines`: `const xge_shader_define_t* pDefines`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iDefineCount`: `int iDefineCount`.
- `ppShader`: `xge_shader* ppShader`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeShaderVariantSetInit`
- `xgeShaderVariantSetFree`

---

### xgeMaterialInit

Initializes the Material object or subsystem.

**Purpose:**

Initializes the Material object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialInit(xge_material pMaterial);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.

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

- `xgeMaterialFree`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`

---

### xgeMaterialFree

Releases resources associated with Material.

**Purpose:**

Releases resources associated with Material. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialFree(xge_material pMaterial);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.

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

- `xgeMaterialInit`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`

---

### xgeMaterialSetShader

Sets Material Shader state or configuration.

**Purpose:**

Sets Material Shader state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialSetShader(xge_material pMaterial, xge_shader pShader);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.
- `pShader`: `xge_shader pShader`.

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

- `xgeMaterialInit`
- `xgeMaterialFree`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`

---

### xgeMaterialSetTexture

Sets Material Texture state or configuration.

**Purpose:**

Sets Material Texture state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialSetTexture(xge_material pMaterial, xge_texture pTexture);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.
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

- `xgeMaterialInit`
- `xgeMaterialFree`
- `xgeMaterialSetShader`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`

---

### xgeMaterialSetColor

Sets Material Color state or configuration.

**Purpose:**

Sets Material Color state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialSetColor(xge_material pMaterial, uint32_t iColor);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.
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

- `xgeMaterialInit`
- `xgeMaterialFree`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`

---

### xgeMaterialSetBlend

Sets Material Blend state or configuration.

**Purpose:**

Sets Material Blend state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialSetBlend(xge_material pMaterial, int iBlend);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.
- `iBlend`: `int iBlend`.

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

- `xgeMaterialInit`
- `xgeMaterialFree`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialDraw`

---

### xgeMaterialDraw

Draws or paints using Material Draw.

**Purpose:**

Draws or paints using Material Draw. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMaterialDraw(xge_material pMaterial, const xge_draw_t* pDraw);
```

**Parameters:**

- `pMaterial`: `xge_material pMaterial`.
- `pDraw`: `const xge_draw_t* pDraw`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeMaterialInit`
- `xgeMaterialFree`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`

---

### xgeMeshCreate

Creates a Mesh resource.

**Purpose:**

Creates a Mesh resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags);
```

**Parameters:**

- `pMesh`: `xge_mesh pMesh`.
- `pVertices`: `const xge_vertex_t* pVertices`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iVertexCount`: `int iVertexCount`.
- `pIndices`: `const uint16_t* pIndices`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iIndexCount`: `int iIndexCount`.
- `iFlags`: `uint32_t iFlags`.

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

- `xgeMeshUpdate`
- `xgeMeshFree`
- `xgeMeshDraw`

---

### xgeMeshUpdate

Updates Mesh Update state.

**Purpose:**

Updates Mesh Update state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount);
```

**Parameters:**

- `pMesh`: `xge_mesh pMesh`.
- `pVertices`: `const xge_vertex_t* pVertices`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iVertexCount`: `int iVertexCount`.
- `pIndices`: `const uint16_t* pIndices`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iIndexCount`: `int iIndexCount`.

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

### xgeMeshFree

Releases resources associated with Mesh.

**Purpose:**

Releases resources associated with Mesh. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMeshFree(xge_mesh pMesh);
```

**Parameters:**

- `pMesh`: `xge_mesh pMesh`.

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

- `xgeMeshCreate`
- `xgeMeshUpdate`
- `xgeMeshDraw`

---

### xgeMeshDraw

Draws or paints using Mesh Draw.

**Purpose:**

Draws or paints using Mesh Draw. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags);
```

**Parameters:**

- `pMesh`: `xge_mesh pMesh`.
- `pTexture`: `xge_texture pTexture`.
- `iFlags`: `uint32_t iFlags`.

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

- `xgeMeshCreate`
- `xgeMeshUpdate`
- `xgeMeshFree`

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
