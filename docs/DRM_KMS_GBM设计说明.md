# DRM/KMS/GBM 设计说明

板卡 Linux 后续显示输出优先考虑 DRM/KMS/GBM + EGL。

## 目标

- 在无桌面环境下创建 GLES3 context。
- 使用 GBM surface 作为 EGL window surface。
- 使用 KMS page flip 输出到显示设备。
- 与 XGE render command / render target 层保持解耦。

## 初始化顺序

```text
open /dev/dri/card*
  -> drmModeGetResources
  -> choose connector/crtc/mode
  -> gbm_create_device
  -> gbm_surface_create
  -> eglGetDisplay(gbm_device)
  -> eglInitialize
  -> eglChooseConfig
  -> eglCreateWindowSurface(gbm_surface)
  -> eglCreateContext(GLES3)
  -> eglMakeCurrent
```

## 呈现顺序

```text
glFlush / eglSwapBuffers
  -> gbm_surface_lock_front_buffer
  -> drmModeAddFB
  -> drmModePageFlip
  -> release previous gbm buffer
```

## 第一版边界

- 不在 XGE core 中暴露 DRM/KMS 细节。
- `xgeEGLInit` 只暴露通用 EGL context 描述。
- 具体 DRM/KMS/GBM 代码后续放入独立平台模块。
- 输入不从 DRM 读取，另走 evdev/Wayland/X11 策略。
