# XGE Offscreen Example

This example tracks offscreen rendering work.

Current implemented path:

- `xgeOffscreenInit`
- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`
- CPU-shadow readback when no GL context is active

Pending real GPU path:

- EGL pbuffer context creation
- EGL surfaceless context creation
- GLES3 function loading on target Linux
- Human smoke test on board Linux
