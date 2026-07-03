# Texture

> Image loading, texture creation, sampler state, fallback textures, upload queues, premultiplied alpha, and pixel readback.

[Back to API Reference](README.en.md) | [Chinese Source](texture.md)

---

## Module Role

Image loading, texture creation, sampler state, fallback textures, upload queues, premultiplied alpha, and pixel readback.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeImageLoad

Loads or opens the Image resource.

**Purpose:**

Loads or opens the Image resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeImageLoad(xge_image pImage, const char* sPath);
```

**Parameters:**

- `pImage`: `xge_image pImage`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`

---

### xgeImageLoadEx

Loads or opens the Image Ex resource.

**Purpose:**

Loads or opens the Image Ex resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags);
```

**Parameters:**

- `pImage`: `xge_image pImage`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
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

- `xgeImageLoad`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`

---

### xgeImageLoadMemory

Loads or opens the Image Memory resource.

**Purpose:**

Loads or opens the Image Memory resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize);
```

**Parameters:**

- `pImage`: `xge_image pImage`.
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

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`

---

### xgeImageLoadMemoryEx

Loads or opens the Image Memory Ex resource.

**Purpose:**

Loads or opens the Image Memory Ex resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags);
```

**Parameters:**

- `pImage`: `xge_image pImage`.
- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.
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

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`

---

### xgeImageGetPixels

Gets Image Pixels state or information.

**Purpose:**

Gets Image Pixels state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void* xgeImageGetPixels(xge_image pImage);
```

**Parameters:**

- `pImage`: `xge_image pImage`.

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

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImagePremultiply`
- `xgeImageFree`

---

### xgeImagePremultiply

Provides the `xgeImagePremultiply` operation.

**Purpose:**

Provides the `xgeImagePremultiply` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeImagePremultiply(xge_image pImage);
```

**Parameters:**

- `pImage`: `xge_image pImage`.

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

### xgeImageFree

Releases resources associated with Image.

**Purpose:**

Releases resources associated with Image. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeImageFree(xge_image pImage);
```

**Parameters:**

- `pImage`: `xge_image pImage`.

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

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`

---

### xgeTextureCreateRGBA

Creates a Texture RGBA resource.

**Purpose:**

Creates a Texture RGBA resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.
- `pPixels`: `const void* pPixels`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`

---

### xgeTextureCreateFromImage

Creates a Texture From Image resource.

**Purpose:**

Creates a Texture From Image resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `pImage`: `const xge_image_t* pImage`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeTextureCreateRGBA`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`

---

### xgeTextureLoad

Loads or opens the Texture resource.

**Purpose:**

Loads or opens the Texture resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`

---

### xgeTextureLoadEx

Loads or opens the Texture Ex resource.

**Purpose:**

Loads or opens the Texture Ex resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`

---

### xgeTextureLoadMemory

Loads or opens the Texture Memory resource.

**Purpose:**

Loads or opens the Texture Memory resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`

---

### xgeTextureLoadMemoryEx

Loads or opens the Texture Memory Ex resource.

**Purpose:**

Loads or opens the Texture Memory Ex resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.
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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureAddRef`

---

### xgeTextureAddRef

Provides the `xgeTextureAddRef` operation.

**Purpose:**

Provides the `xgeTextureAddRef` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureAddRef(xge_texture pTexture);
```

**Parameters:**

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

- None.

---

### xgeTextureUpdateRGBA

Updates Texture Update RGBA state.

**Purpose:**

Updates Texture Update RGBA state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `iX`: `int iX`.
- `iY`: `int iY`.
- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.
- `pPixels`: `const void* pPixels`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
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

### xgeSamplerDefault

Provides the `xgeSamplerDefault` operation.

**Purpose:**

Provides the `xgeSamplerDefault` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_sampler_t xgeSamplerDefault(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_sampler_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

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

### xgeTextureSetSampler

Sets Texture Sampler state or configuration.

**Purpose:**

Sets Texture Sampler state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `pSampler`: `const xge_sampler_t* pSampler`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`

---

### xgeTextureGetSampler

Gets Texture Sampler state or information.

**Purpose:**

Gets Texture Sampler state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_sampler_t xgeTextureGetSampler(xge_texture pTexture);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.

**Return Value:**

- Returns a `xge_sampler_t` value.

**Ownership:**

Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.

**Notes:**

- Use the exact prototype above when declaring or binding this function.
- Prefer checking return codes for functions that return `int`.
- For backend-specific behavior, check the compatibility and platform documentation.

**Example:**

```c
/* See the module guide and case documents for complete runnable examples. */
```

**Related APIs:**

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`

---

### xgeTextureFallbackSetRGBA

Sets Texture Fallback RGBA state or configuration.

**Purpose:**

Sets Texture Fallback RGBA state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels);
```

**Parameters:**

- `iWidth`: `int iWidth`.
- `iHeight`: `int iHeight`.
- `pPixels`: `const void* pPixels`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeTextureFallbackGet`
- `xgeTextureFallbackClear`

---

### xgeTextureFallbackGet

Gets Texture Fallback Get state or information.

**Purpose:**

Gets Texture Fallback Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureFallbackGet(xge_texture pTexture);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.

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

- `xgeTextureFallbackSetRGBA`
- `xgeTextureFallbackClear`

---

### xgeTextureFallbackClear

Clears or resets Texture Fallback Clear state.

**Purpose:**

Clears or resets Texture Fallback Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeTextureFallbackClear(void);
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

### xgeTextureUploadQueue

Loads or opens the Texture Upload Queue resource.

**Purpose:**

Loads or opens the Texture Upload Queue resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureUploadQueue(xge_texture pTexture);
```

**Parameters:**

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

- None.

---

### xgeTextureUploadFlush

Loads or opens the Texture Upload Flush resource.

**Purpose:**

Loads or opens the Texture Upload Flush resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureUploadFlush(void);
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

### xgeTextureReadPixels

Provides the `xgeTextureReadPixels` operation.

**Purpose:**

Provides the `xgeTextureReadPixels` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
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

### xgeTextureFree

Releases resources associated with Texture.

**Purpose:**

Releases resources associated with Texture. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeTextureFree(xge_texture pTexture);
```

**Parameters:**

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

- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`

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
