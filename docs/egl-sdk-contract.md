# Optional EGL SDK

For the subsequent complete GL startup transaction and checked, retryable raw
EGL teardown contract, see [graphics context safety](graphics-context-startup.md).
Its additional cleanup-duty flag requires a newly matched SDK/runtime pair.

`XGE_ENABLE_EGL` is an explicit 0/1 build feature, defaulting to 0 in all normal
profiles. Bit 10 of `XGE_FEATURES` records it, and the shared SDK configuration
and ABI fingerprint include it. A source-only `XGE_HAS_EGL` selects the same bit
when no SDK override exists; it cannot override a disabled matched SDK.

The qualified optional target uses the Linux system EGL development SDK:

```sh
python3 -B tools/build/build.py default build/egl-host --host-xrt --define XGE_ENABLE_EGL=1
python3 -B tools/build/static.py --root . --out build/egl-static --define XGE_ENABLE_EGL=1
```

The builder resolves `pkg-config egl`, its authoritative headers and actual
compiler link inputs before compiling engine TUs. It records their hashes and
verifies them again before accepting the build. Static metadata includes EGL
among the required system libraries; dynamic metadata records the same SDK.
No EGL dispatcher or driver bytes are copied into the distribution. They must
remain available on the target system. Non-system library paths/options need
their own deployment qualification and are explicitly rejected by this target.

The normal Windows OpenGL target still builds without EGL or pkg-config. An
explicit Windows EGL selection currently fails before compiling TUs because no
matching Windows EGL SDK/runtime deployment has been qualified. This does not
silently disable the requested feature or claim that Windows EGL is delivered.

Default Windows and Linux SDKs each pass 12 shared/static build/run checks.
The enabled Linux SDK passes 49: actual PBuffer, Surfaceless and X11 native
window contexts; GLES FBO pixels; native surface swap; seven context startup
failure stages plus window-surface failure; exact backend preservation and
resource cleanup. Typed ELF interposition reaches the actual shared image and
the actual archive, rather than compiling a replacement EGL TU.

Ten further runtime checks use no DISPLAY or WAYLAND_DISPLAY and explicitly
select Mesa `EGL_PLATFORM=surfaceless`: native PBuffer/Surfaceless GLES pixels
and language JIT/static raw-context fixtures pass. The Xvfb/Mesa and explicit
headless paths qualify software backend behavior, not hardware performance.

This is a raw context/SDK contract. `xgeEGLInit` alone does not attach the XGE
GL function table or renderer, provide independent code ownership, or preserve
GPU children across context retirement. An actual enabled-SDK control confirms
that old `xgeOffscreenInit` creates EGL while its target still has zero GPU
texture/FBO IDs. Owning context/renderer sessions and host callback dispatch
remain required before exposing this as a safe language resource.

In this frozen GCC cohort, enabling EGL adds 128 bytes to the stripped Linux
shared image and 6,416 bytes to the static archive. Default Windows/Linux image
sizes are unchanged. These are file-size observations, not per-program size or
speed promises; system driver loading and final linker selection remain costs.

Evidence: `docs/validation/egl-sdk-20261110.json`.
