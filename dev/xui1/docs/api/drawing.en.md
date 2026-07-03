# Drawing

> Immediate drawing, sprite batching, shape drawing, and lightweight 2.5D drawing.

[Back to API Reference](README.en.md) | [Chinese Source](drawing.md)

---

## Module Role

Immediate drawing, sprite batching, shape drawing, and lightweight 2.5D drawing.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeDraw

Draws or paints using Draw.

**Purpose:**

Draws or paints using Draw. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
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

- `xgeDrawEx`
- `xgeDrawQuad3D`
- `xgeDrawPx`

---

### xgeDrawEx

Draws or paints using Draw Ex.

**Purpose:**

Draws or paints using Draw Ex. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeDrawEx(const xge_draw_t* pDraw);
```

**Parameters:**

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

- None.

---

### xgeDrawQuad3D

Draws or paints using Draw Quad3 D.

**Purpose:**

Draws or paints using Draw Quad3 D. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `pVertices`: `const xge_vertex_t* pVertices`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
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

- None.

---

### xgeDrawPx

Draws or paints using Draw Px.

**Purpose:**

Draws or paints using Draw Px. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);
```

**Parameters:**

- `pTexture`: `xge_texture pTexture`.
- `iX`: `int iX`.
- `iY`: `int iY`.

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

### xgeSpriteBatchInit

Initializes the Sprite Batch object or subsystem.

**Purpose:**

Initializes the Sprite Batch object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSpriteBatchInit(xge_sprite_batch pBatch, xge_texture pTexture, int iCapacity, uint32_t iFlags);
```

**Parameters:**

- `pBatch`: `xge_sprite_batch pBatch`.
- `pTexture`: `xge_texture pTexture`.
- `iCapacity`: `int iCapacity`.
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

- `xgeSpriteBatchFree`
- `xgeSpriteBatchClear`
- `xgeSpriteBatchAdd`
- `xgeSpriteBatchFlush`

---

### xgeSpriteBatchFree

Releases resources associated with Sprite Batch.

**Purpose:**

Releases resources associated with Sprite Batch. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSpriteBatchFree(xge_sprite_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_sprite_batch pBatch`.

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

- `xgeSpriteBatchInit`
- `xgeSpriteBatchClear`
- `xgeSpriteBatchAdd`
- `xgeSpriteBatchFlush`

---

### xgeSpriteBatchClear

Clears or resets Sprite Batch Clear state.

**Purpose:**

Clears or resets Sprite Batch Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSpriteBatchClear(xge_sprite_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_sprite_batch pBatch`.

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

### xgeSpriteBatchAdd

Provides the `xgeSpriteBatchAdd` operation.

**Purpose:**

Provides the `xgeSpriteBatchAdd` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSpriteBatchAdd(xge_sprite_batch pBatch, const xge_draw_t* pDraw);
```

**Parameters:**

- `pBatch`: `xge_sprite_batch pBatch`.
- `pDraw`: `const xge_draw_t* pDraw`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

### xgeSpriteBatchFlush

Provides the `xgeSpriteBatchFlush` operation.

**Purpose:**

Provides the `xgeSpriteBatchFlush` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSpriteBatchFlush(xge_sprite_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_sprite_batch pBatch`.

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

### xgeShapePoint

Provides the `xgeShapePoint` operation.

**Purpose:**

Provides the `xgeShapePoint` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fSize`: `float fSize`.
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

- `xgeShapePointPx`

---

### xgeShapePointPx

Provides the `xgeShapePointPx` operation.

**Purpose:**

Provides the `xgeShapePointPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fSize`: `float fSize`.
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

- None.

---

### xgeShapeLine

Provides the `xgeShapeLine` operation.

**Purpose:**

Provides the `xgeShapeLine` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX0`: `float fX0`.
- `fY0`: `float fY0`.
- `fX1`: `float fX1`.
- `fY1`: `float fY1`.
- `fWidth`: `float fWidth`.
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

- `xgeShapeLinePx`

---

### xgeShapeLinePx

Provides the `xgeShapeLinePx` operation.

**Purpose:**

Provides the `xgeShapeLinePx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX0`: `float fX0`.
- `fY0`: `float fY0`.
- `fX1`: `float fX1`.
- `fY1`: `float fY1`.
- `fWidth`: `float fWidth`.
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

- None.

---

### xgeShapeRectFill

Provides the `xgeShapeRectFill` operation.

**Purpose:**

Provides the `xgeShapeRectFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);
```

**Parameters:**

- `tRect`: `xge_rect_t tRect`.
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

- `xgeShapeRectFillPx`

---

### xgeShapeRectFillPx

Provides the `xgeShapeRectFillPx` operation.

**Purpose:**

Provides the `xgeShapeRectFillPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);
```

**Parameters:**

- `tRect`: `xge_rect_t tRect`.
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

- None.

---

### xgeShapeRectStroke

Provides the `xgeShapeRectStroke` operation.

**Purpose:**

Provides the `xgeShapeRectStroke` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);
```

**Parameters:**

- `tRect`: `xge_rect_t tRect`.
- `fWidth`: `float fWidth`.
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

- `xgeShapeRectStrokePx`

---

### xgeShapeRectStrokePx

Provides the `xgeShapeRectStrokePx` operation.

**Purpose:**

Provides the `xgeShapeRectStrokePx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);
```

**Parameters:**

- `tRect`: `xge_rect_t tRect`.
- `fWidth`: `float fWidth`.
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

- None.

---

### xgeShapeCircleFill

Provides the `xgeShapeCircleFill` operation.

**Purpose:**

Provides the `xgeShapeCircleFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
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

- `xgeShapeCircleFillPx`

---

### xgeShapeCircleFillPx

Provides the `xgeShapeCircleFillPx` operation.

**Purpose:**

Provides the `xgeShapeCircleFillPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
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

- None.

---

### xgeShapeCircleStroke

Provides the `xgeShapeCircleStroke` operation.

**Purpose:**

Provides the `xgeShapeCircleStroke` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
- `fWidth`: `float fWidth`.
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

- `xgeShapeCircleStrokePx`

---

### xgeShapeCircleStrokePx

Provides the `xgeShapeCircleStrokePx` operation.

**Purpose:**

Provides the `xgeShapeCircleStrokePx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
- `fWidth`: `float fWidth`.
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

- None.

---

### xgeShapeArc

Provides the `xgeShapeArc` operation.

**Purpose:**

Provides the `xgeShapeArc` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
- `fStartRadians`: `float fStartRadians`.
- `fEndRadians`: `float fEndRadians`.
- `fWidth`: `float fWidth`.
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

- `xgeShapeArcPx`

---

### xgeShapeArcPx

Provides the `xgeShapeArcPx` operation.

**Purpose:**

Provides the `xgeShapeArcPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
```

**Parameters:**

- `fX`: `float fX`.
- `fY`: `float fY`.
- `fRadius`: `float fRadius`.
- `fStartRadians`: `float fStartRadians`.
- `fEndRadians`: `float fEndRadians`.
- `fWidth`: `float fWidth`.
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

- None.

---

### xgeShapeTriangleFill

Provides the `xgeShapeTriangleFill` operation.

**Purpose:**

Provides the `xgeShapeTriangleFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
```

**Parameters:**

- `tA`: `xge_vec2_t tA`.
- `tB`: `xge_vec2_t tB`.
- `tC`: `xge_vec2_t tC`.
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

- `xgeShapeTriangleFillPx`

---

### xgeShapeTriangleFillPx

Provides the `xgeShapeTriangleFillPx` operation.

**Purpose:**

Provides the `xgeShapeTriangleFillPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
```

**Parameters:**

- `tA`: `xge_vec2_t tA`.
- `tB`: `xge_vec2_t tB`.
- `tC`: `xge_vec2_t tC`.
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

- None.

---

### xgeShapePolygonFill

Provides the `xgeShapePolygonFill` operation.

**Purpose:**

Provides the `xgeShapePolygonFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
```

**Parameters:**

- `pPoints`: `const xge_vec2_t* pPoints`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iCount`: `int iCount`.
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

- `xgeShapePolygonFillPx`

---

### xgeShapePolygonFillPx

Provides the `xgeShapePolygonFillPx` operation.

**Purpose:**

Provides the `xgeShapePolygonFillPx` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
```

**Parameters:**

- `pPoints`: `const xge_vec2_t* pPoints`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `iCount`: `int iCount`.
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

- None.

---

### xgeShapeBatchInit

Initializes the Shape Batch object or subsystem.

**Purpose:**

Initializes the Shape Batch object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.
- `iColor`: `uint32_t iColor`.
- `iTriangleCapacity`: `int iTriangleCapacity`.
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

- `xgeShapeBatchFree`
- `xgeShapeBatchClear`
- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFlush`

---

### xgeShapeBatchFree

Releases resources associated with Shape Batch.

**Purpose:**

Releases resources associated with Shape Batch. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeBatchFree(xge_shape_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.

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

- `xgeShapeBatchInit`
- `xgeShapeBatchClear`
- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFlush`

---

### xgeShapeBatchClear

Clears or resets Shape Batch Clear state.

**Purpose:**

Clears or resets Shape Batch Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeShapeBatchClear(xge_shape_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.

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

### xgeShapeBatchTriangleFill

Provides the `xgeShapeBatchTriangleFill` operation.

**Purpose:**

Provides the `xgeShapeBatchTriangleFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.
- `tA`: `xge_vec2_t tA`.
- `tB`: `xge_vec2_t tB`.
- `tC`: `xge_vec2_t tC`.

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

### xgeShapeBatchRectFill

Provides the `xgeShapeBatchRectFill` operation.

**Purpose:**

Provides the `xgeShapeBatchRectFill` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.
- `tRect`: `xge_rect_t tRect`.

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

### xgeShapeBatchFlush

Provides the `xgeShapeBatchFlush` operation.

**Purpose:**

Provides the `xgeShapeBatchFlush` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeShapeBatchFlush(xge_shape_batch pBatch);
```

**Parameters:**

- `pBatch`: `xge_shape_batch pBatch`.

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
