# Font Text

> UTF-8 text, TrueType fonts, XRF bitmap font caches, glyph access, fallback fonts, and text drawing.

[Back to API Reference](README.en.md) | [Chinese Source](font-text.md)

---

## Module Role

UTF-8 text, TrueType fonts, XRF bitmap font caches, glyph access, fallback fonts, and text drawing.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeTextUTF8Next

Provides the `xgeTextUTF8Next` operation.

**Purpose:**

Provides the `xgeTextUTF8Next` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint);
```

**Parameters:**

- `psText`: `const char** psText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pCodepoint`: `uint32_t* pCodepoint`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeFontLoad

Loads or opens the Font resource.

**Purpose:**

Loads or opens the Font resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontLoad(xge_font pFont, const char* sPath, float fSize);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fSize`: `float fSize`.

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

- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`

---

### xgeFontLoadMemory

Loads or opens the Font Memory resource.

**Purpose:**

Loads or opens the Font Memory resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.
- `fSize`: `float fSize`.

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

- `xgeFontLoad`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`

---

### xgeFontLoadXRF

Loads or opens the Font XRF resource.

**Purpose:**

Loads or opens the Font XRF resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontLoadXRF(xge_font pFont, const char* sPath);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
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

- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`

---

### xgeFontLoadXRFMemory

Loads or opens the Font XRFMemory resource.

**Purpose:**

Loads or opens the Font XRFMemory resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
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

- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`

---

### xgeFontBuildXRFMemory

Provides the `xgeFontBuildXRFMemory` operation.

**Purpose:**

Provides the `xgeFontBuildXRFMemory` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `iFirstCodepoint`: `uint32_t iFirstCodepoint`.
- `iCount`: `uint32_t iCount`.
- `ppData`: `void** ppData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pSize`: `int* pSize`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeFontSaveXRF

Provides the `xgeFontSaveXRF` operation.

**Purpose:**

Provides the `xgeFontSaveXRF` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iFirstCodepoint`: `uint32_t iFirstCodepoint`.
- `iCount`: `uint32_t iCount`.

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

### xgeFontLoadCached

Loads or opens the Font Cached resource.

**Purpose:**

Loads or opens the Font Cached resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sTTFPath`: `const char* sTTFPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `sXRFPath`: `const char* sXRFPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fSize`: `float fSize`.
- `iFirstCodepoint`: `uint32_t iFirstCodepoint`.
- `iCount`: `uint32_t iCount`.

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

- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`

---

### xgeFontAddRef

Provides the `xgeFontAddRef` operation.

**Purpose:**

Provides the `xgeFontAddRef` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontAddRef(xge_font pFont);
```

**Parameters:**

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

- None.

---

### xgeFontFree

Releases resources associated with Font.

**Purpose:**

Releases resources associated with Font. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeFontFree(xge_font pFont);
```

**Parameters:**

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

- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`

---

### xgeFontSetFallback

Sets Font Fallback state or configuration.

**Purpose:**

Sets Font Fallback state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `pFallback`: `xge_font pFallback`.

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

- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`

---

### xgeFontFallbackSet

Sets Font Fallback Set state or configuration.

**Purpose:**

Sets Font Fallback Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontFallbackSet(const char* sPath, float fSize);
```

**Parameters:**

- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fSize`: `float fSize`.

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

- `xgeFontFallbackSetMemory`
- `xgeFontFallbackGet`
- `xgeFontFallbackClear`

---

### xgeFontFallbackSetMemory

Sets Font Fallback Memory state or configuration.

**Purpose:**

Sets Font Fallback Memory state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize);
```

**Parameters:**

- `pData`: `const void* pData`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iSize`: `int iSize`.
- `fSize`: `float fSize`.

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

- `xgeFontFallbackSet`
- `xgeFontFallbackGet`
- `xgeFontFallbackClear`

---

### xgeFontFallbackGet

Gets Font Fallback Get state or information.

**Purpose:**

Gets Font Fallback Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontFallbackGet(xge_font pFont, float fSize);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `fSize`: `float fSize`.

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

- `xgeFontFallbackSet`
- `xgeFontFallbackSetMemory`
- `xgeFontFallbackClear`

---

### xgeFontFallbackClear

Clears or resets Font Fallback Clear state.

**Purpose:**

Clears or resets Font Fallback Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeFontFallbackClear(void);
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

### xgeFontGlyphGet

Gets Font Glyph Get state or information.

**Purpose:**

Gets Font Glyph Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `iCodepoint`: `uint32_t iCodepoint`.
- `pMetrics`: `xge_glyph_metrics_t* pMetrics`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeFontGlyphRasterize`
- `xgeFontGlyphAtlasGet`

---

### xgeFontGlyphRasterize

Provides the `xgeFontGlyphRasterize` operation.

**Purpose:**

Provides the `xgeFontGlyphRasterize` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `iCodepoint`: `uint32_t iCodepoint`.
- `pBitmap`: `xge_glyph_bitmap_t* pBitmap`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeFontGlyphAtlasGet

Gets Font Glyph Atlas Get state or information.

**Purpose:**

Gets Font Glyph Atlas Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `iCodepoint`: `uint32_t iCodepoint`.
- `pGlyph`: `xge_glyph_t* pGlyph`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeGlyphBitmapFree

Releases resources associated with Glyph Bitmap.

**Purpose:**

Releases resources associated with Glyph Bitmap. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap);
```

**Parameters:**

- `pBitmap`: `xge_glyph_bitmap_t* pBitmap`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeTextMeasure

Provides the `xgeTextMeasure` operation.

**Purpose:**

Provides the `xgeTextMeasure` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeTextDraw

Draws or paints using Text Draw.

**Purpose:**

Draws or paints using Text Draw. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `fX`: `float fX`.
- `fY`: `float fY`.
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

- `xgeTextUTF8Next`
- `xgeTextMeasure`
- `xgeTextDrawRect`

---

### xgeTextDrawRect

Draws or paints using Text Draw Rect.

**Purpose:**

Draws or paints using Text Draw Rect. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags);
```

**Parameters:**

- `pFont`: `xge_font pFont`.
- `sText`: `const char* sText`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `tRect`: `xge_rect_t tRect`.
- `iColor`: `uint32_t iColor`.
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

- `xgeTextUTF8Next`
- `xgeTextMeasure`
- `xgeTextDraw`

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
