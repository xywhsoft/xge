# 板卡 Linux 入门

本教程说明 XGE 在现代板卡 Linux 上的目标路径。

[返回教程索引](README.md) | [离屏教程](offscreen-intro.md) | [平台冒烟范例](../case/platform-smoke.md)

## 目标范围

第一版只考虑近几年较新的板卡，例如 RK3566 这类具备 GLES3/EGL 能力的平台。老旧 GLES2 设备不是当前兼容基线。

## 推荐路径

```text
EGL
  -> GBM / DRM / KMS 或 pbuffer / surfaceless
  -> GLES3 context
  -> XGE render thread
```

有桌面环境时可以走 Sokol X11/Wayland；无桌面环境时走 EGL/GBM/KMS 或离屏。

## 输入

板卡输入可能来自 evdev、触摸屏、键盘、鼠标或宿主程序。平台后端应把它们统一转换成 XGE 输入事件。

## 验证重点

- EGL 初始化是否成功。
- GLES 版本是否达到 3.0。
- framebuffer 格式是否正确。
- swap/present 或 readback 是否正常。
- 输入设备权限是否正确。
- render thread 是否真正拥有 context。

## 常见错误

不要只检查 OpenGL 包名。板卡关键是 EGL/GLES 驱动和 display/surface 路径。

不要默认有 X11/Wayland。很多板卡部署环境只有 DRM/KMS。

不要把 root 权限作为唯一方案。输入设备和 DRM 权限应通过 udev/group 处理。

## 下一步

- 离屏路径读 [离屏渲染入门](offscreen-intro.md)。
- 渲染线程读 [专用 Render Thread 入门](render-thread-intro.md)。
