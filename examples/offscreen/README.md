# XGE Offscreen Example

This example tracks offscreen rendering work.

Current implemented path:

- `xgeOffscreenInit`
- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`
- CPU-shadow readback when no GL context is active
- guarded EGL pbuffer context creation when built with `XGE_HAS_EGL` and `XGE_HAS_EGL_PBUFFER`
- `xge_offscreen_t::bEGLContext` / `bFallbackRenderTarget` distinguish real EGL context from CPU-side fallback
- `examples/offscreen/main.c` board Linux pbuffer smoke entry

Pending real GPU path:

- GLES3 function loading on target Linux
- Human smoke test on board Linux

Board Linux build:

```sh
./examples/offscreen/build_board_linux_egl.sh
./build/xge_board_linux_egl
```

Surfaceless EGL build:

```sh
./examples/egl_surfaceless/build.sh
./build/xge_egl_surfaceless
```

`examples/egl_surfaceless/build.sh` enables `XGE_HAS_EGL_SURFACELESS`, but the target driver must expose `EGL_KHR_surfaceless_context`; otherwise the example reports unsupported or backend failure.
