# Audio

> Audio initialization, sound, music, streaming, mixer groups, fading, and 3D listener state.

[Back to API Reference](README.en.md) | [Chinese Source](audio.md)

---

## Module Role

Audio initialization, sound, music, streaming, mixer groups, fading, and 3D listener state.

This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.

## Call Order

Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.

## Function Reference

### xgeAudioInit

Initializes the Audio object or subsystem.

**Purpose:**

Initializes the Audio object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAudioInit(void);
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

- `xgeAudioUnit`
- `xgeAudioIsReady`
- `xgeAudioSetVolume`
- `xgeAudioGetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`

---

### xgeAudioUnit

Releases resources associated with Audio.

**Purpose:**

Releases resources associated with Audio. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioUnit(void);
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

- `xgeAudioInit`
- `xgeAudioIsReady`
- `xgeAudioSetVolume`
- `xgeAudioGetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`

---

### xgeAudioIsReady

Provides the `xgeAudioIsReady` operation.

**Purpose:**

Provides the `xgeAudioIsReady` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAudioIsReady(void);
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

- None.

---

### xgeAudioSetVolume

Sets Audio Volume state or configuration.

**Purpose:**

Sets Audio Volume state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioSetVolume(float fVolume);
```

**Parameters:**

- `fVolume`: `float fVolume`.

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

- `xgeAudioInit`
- `xgeAudioUnit`
- `xgeAudioIsReady`
- `xgeAudioGetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`

---

### xgeAudioGetVolume

Gets Audio Volume state or information.

**Purpose:**

Gets Audio Volume state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeAudioGetVolume(void);
```

**Parameters:**

None.

**Return Value:**

- Returns the requested numeric value. Invalid inputs generally return `0`.

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

- `xgeAudioInit`
- `xgeAudioUnit`
- `xgeAudioIsReady`
- `xgeAudioSetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`

---

### xgeAudioListenerSet

Sets Audio Listener Set state or configuration.

**Purpose:**

Sets Audio Listener Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioListenerSet(const xge_audio_listener_t* pListener);
```

**Parameters:**

- `pListener`: `const xge_audio_listener_t* pListener`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.

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

- `xgeAudioListenerGet`

---

### xgeAudioListenerGet

Gets Audio Listener Get state or information.

**Purpose:**

Gets Audio Listener Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API xge_audio_listener_t xgeAudioListenerGet(void);
```

**Parameters:**

None.

**Return Value:**

- Returns a `xge_audio_listener_t` value.

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

- `xgeAudioListenerSet`

---

### xgeAudioGroupInit

Initializes the Audio Group object or subsystem.

**Purpose:**

Initializes the Audio Group object or subsystem. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeAudioGroupInit(xge_audio_group pGroup);
```

**Parameters:**

- `pGroup`: `xge_audio_group pGroup`.

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

- `xgeAudioGroupFree`
- `xgeAudioGroupSetVolume`
- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`

---

### xgeAudioGroupFree

Releases resources associated with Audio Group.

**Purpose:**

Releases resources associated with Audio Group. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioGroupFree(xge_audio_group pGroup);
```

**Parameters:**

- `pGroup`: `xge_audio_group pGroup`.

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

- `xgeAudioGroupInit`
- `xgeAudioGroupSetVolume`
- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`

---

### xgeAudioGroupSetVolume

Sets Audio Group Volume state or configuration.

**Purpose:**

Sets Audio Group Volume state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume);
```

**Parameters:**

- `pGroup`: `xge_audio_group pGroup`.
- `fVolume`: `float fVolume`.

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

- `xgeAudioGroupInit`
- `xgeAudioGroupFree`
- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`

---

### xgeAudioGroupGetVolume

Gets Audio Group Volume state or information.

**Purpose:**

Gets Audio Group Volume state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API float xgeAudioGroupGetVolume(xge_audio_group pGroup);
```

**Parameters:**

- `pGroup`: `xge_audio_group pGroup`.

**Return Value:**

- Returns the requested numeric value. Invalid inputs generally return `0`.

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

- `xgeAudioGroupInit`
- `xgeAudioGroupFree`
- `xgeAudioGroupSetVolume`
- `xgeAudioGroupFade`

---

### xgeAudioGroupFade

Provides the `xgeAudioGroupFade` operation.

**Purpose:**

Provides the `xgeAudioGroupFade` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds);
```

**Parameters:**

- `pGroup`: `xge_audio_group pGroup`.
- `fFrom`: `float fFrom`.
- `fTo`: `float fTo`.
- `iMilliseconds`: `int iMilliseconds`.

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

### xgeSoundLoad

Loads or opens the Sound resource.

**Purpose:**

Loads or opens the Sound resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundLoad(xge_sound pSound, const char* sPath);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
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

- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`
- `xgeSoundStop`

---

### xgeSoundLoadGroup

Loads or opens the Sound Group resource.

**Purpose:**

Loads or opens the Sound Group resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pGroup`: `xge_audio_group pGroup`.

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

- `xgeSoundLoad`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`
- `xgeSoundStop`

---

### xgeSoundFallbackSet

Sets Sound Fallback Set state or configuration.

**Purpose:**

Sets Sound Fallback Set state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundFallbackSet(const char* sPath);
```

**Parameters:**

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

- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`

---

### xgeSoundFallbackGet

Gets Sound Fallback Get state or information.

**Purpose:**

Gets Sound Fallback Get state or information. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundFallbackGet(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

- `xgeSoundFallbackSet`
- `xgeSoundFallbackClear`

---

### xgeSoundFallbackClear

Clears or resets Sound Fallback Clear state.

**Purpose:**

Clears or resets Sound Fallback Clear state. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundFallbackClear(void);
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

### xgeSoundPlay

Provides the `xgeSoundPlay` operation.

**Purpose:**

Provides the `xgeSoundPlay` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundPlay(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeSoundStop

Provides the `xgeSoundStop` operation.

**Purpose:**

Provides the `xgeSoundStop` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundStop(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeSoundPause

Provides the `xgeSoundPause` operation.

**Purpose:**

Provides the `xgeSoundPause` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundPause(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeSoundResume

Provides the `xgeSoundResume` operation.

**Purpose:**

Provides the `xgeSoundResume` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundResume(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeSoundAddRef

Provides the `xgeSoundAddRef` operation.

**Purpose:**

Provides the `xgeSoundAddRef` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundAddRef(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeSoundFree

Releases resources associated with Sound.

**Purpose:**

Releases resources associated with Sound. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundFree(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`

---

### xgeSoundSetLoop

Sets Sound Loop state or configuration.

**Purpose:**

Sets Sound Loop state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundSetLoop(xge_sound pSound, int bLoop);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
- `bLoop`: `int bLoop`.

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

- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`

---

### xgeSoundSetVolume

Sets Sound Volume state or configuration.

**Purpose:**

Sets Sound Volume state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundSetVolume(xge_sound pSound, float fVolume);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
- `fVolume`: `float fVolume`.

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

- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`

---

### xgeSoundSetPosition

Sets Sound Position state or configuration.

**Purpose:**

Sets Sound Position state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
- `fX`: `float fX`.
- `fY`: `float fY`.
- `fZ`: `float fZ`.

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

- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`

---

### xgeSoundFade

Provides the `xgeSoundFade` operation.

**Purpose:**

Provides the `xgeSoundFade` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.
- `fFrom`: `float fFrom`.
- `fTo`: `float fTo`.
- `iMilliseconds`: `int iMilliseconds`.

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

### xgeSoundIsPlaying

Provides the `xgeSoundIsPlaying` operation.

**Purpose:**

Provides the `xgeSoundIsPlaying` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeSoundIsPlaying(xge_sound pSound);
```

**Parameters:**

- `pSound`: `xge_sound pSound`.

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

### xgeMusicLoad

Loads or opens the Music resource.

**Purpose:**

Loads or opens the Music resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicLoad(xge_music pMusic, const char* sPath);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.
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

- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`
- `xgeMusicFree`

---

### xgeMusicLoadGroup

Loads or opens the Music Group resource.

**Purpose:**

Loads or opens the Music Group resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pGroup`: `xge_audio_group pGroup`.

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

- `xgeMusicLoad`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`
- `xgeMusicFree`

---

### xgeMusicPlay

Provides the `xgeMusicPlay` operation.

**Purpose:**

Provides the `xgeMusicPlay` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicPlay(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

### xgeMusicStop

Provides the `xgeMusicStop` operation.

**Purpose:**

Provides the `xgeMusicStop` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicStop(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

### xgeMusicPause

Provides the `xgeMusicPause` operation.

**Purpose:**

Provides the `xgeMusicPause` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicPause(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

### xgeMusicResume

Provides the `xgeMusicResume` operation.

**Purpose:**

Provides the `xgeMusicResume` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicResume(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

### xgeMusicFree

Releases resources associated with Music.

**Purpose:**

Releases resources associated with Music. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMusicFree(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

- `xgeMusicLoad`
- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`

---

### xgeMusicSetLoop

Sets Music Loop state or configuration.

**Purpose:**

Sets Music Loop state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMusicSetLoop(xge_music pMusic, int bLoop);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.
- `bLoop`: `int bLoop`.

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

- `xgeMusicLoad`
- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`

---

### xgeMusicSetVolume

Sets Music Volume state or configuration.

**Purpose:**

Sets Music Volume state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMusicSetVolume(xge_music pMusic, float fVolume);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.
- `fVolume`: `float fVolume`.

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

- `xgeMusicLoad`
- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`

---

### xgeMusicFade

Provides the `xgeMusicFade` operation.

**Purpose:**

Provides the `xgeMusicFade` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.
- `fFrom`: `float fFrom`.
- `fTo`: `float fTo`.
- `iMilliseconds`: `int iMilliseconds`.

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

### xgeMusicIsPlaying

Provides the `xgeMusicIsPlaying` operation.

**Purpose:**

Provides the `xgeMusicIsPlaying` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeMusicIsPlaying(xge_music pMusic);
```

**Parameters:**

- `pMusic`: `xge_music pMusic`.

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

### xgeStreamOpen

Loads or opens the Stream resource.

**Purpose:**

Loads or opens the Stream resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamOpen(xge_stream pStream, const char* sPath);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
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

- `xgeStreamOpenGroup`

---

### xgeStreamOpenGroup

Loads or opens the Stream Group resource.

**Purpose:**

Loads or opens the Stream Group resource. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
- `sPath`: `const char* sPath`. Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise.
- `pGroup`: `xge_audio_group pGroup`.

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

### xgeStreamPlay

Provides the `xgeStreamPlay` operation.

**Purpose:**

Provides the `xgeStreamPlay` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamPlay(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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

### xgeStreamStop

Provides the `xgeStreamStop` operation.

**Purpose:**

Provides the `xgeStreamStop` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamStop(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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

### xgeStreamPause

Provides the `xgeStreamPause` operation.

**Purpose:**

Provides the `xgeStreamPause` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamPause(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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

### xgeStreamResume

Provides the `xgeStreamResume` operation.

**Purpose:**

Provides the `xgeStreamResume` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamResume(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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

### xgeStreamClose

Releases resources associated with Stream.

**Purpose:**

Releases resources associated with Stream. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeStreamClose(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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

### xgeStreamSetLoop

Sets Stream Loop state or configuration.

**Purpose:**

Sets Stream Loop state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeStreamSetLoop(xge_stream pStream, int bLoop);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
- `bLoop`: `int bLoop`.

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

- `xgeStreamOpen`
- `xgeStreamOpenGroup`
- `xgeStreamPlay`
- `xgeStreamStop`
- `xgeStreamPause`
- `xgeStreamResume`

---

### xgeStreamSetVolume

Sets Stream Volume state or configuration.

**Purpose:**

Sets Stream Volume state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeStreamSetVolume(xge_stream pStream, float fVolume);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
- `fVolume`: `float fVolume`.

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

- `xgeStreamOpen`
- `xgeStreamOpenGroup`
- `xgeStreamPlay`
- `xgeStreamStop`
- `xgeStreamPause`
- `xgeStreamResume`

---

### xgeStreamSetPosition

Sets Stream Position state or configuration.

**Purpose:**

Sets Stream Position state or configuration. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
- `fX`: `float fX`.
- `fY`: `float fY`.
- `fZ`: `float fZ`.

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

- `xgeStreamOpen`
- `xgeStreamOpenGroup`
- `xgeStreamPlay`
- `xgeStreamStop`
- `xgeStreamPause`
- `xgeStreamResume`

---

### xgeStreamFade

Provides the `xgeStreamFade` operation.

**Purpose:**

Provides the `xgeStreamFade` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.
- `fFrom`: `float fFrom`.
- `fTo`: `float fTo`.
- `iMilliseconds`: `int iMilliseconds`.

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

### xgeStreamIsPlaying

Provides the `xgeStreamIsPlaying` operation.

**Purpose:**

Provides the `xgeStreamIsPlaying` operation. It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.

**Prototype:**

```c
XGE_API int xgeStreamIsPlaying(xge_stream pStream);
```

**Parameters:**

- `pStream`: `xge_stream pStream`.

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
