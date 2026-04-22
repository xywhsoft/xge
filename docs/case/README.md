# XGE 范例解析

> 面向“多个模块如何串成一个完整程序”的案例入口。这里与 `guide/` 的区别是：`guide/` 先建立使用心智，`case/` 直接展示完整问题的解决方案。

[返回文档中心](../README.md)

---

## 主线必读

第一次系统阅读范例时，建议先按下面顺序走：

1. [最小窗口和 shape 绘制](minimal-window.md)
2. [纹理和 Sprite 绘制](texture-sprite.md)
3. [场景栈](scene-stack.md)
4. [RenderTarget 与离屏绘制](render-target.md)
5. [Shader / Material 效果](shader-material.md)
6. [2.5D 透视 quad](perspective-quad.md)
7. [音频播放和 3D 声源](audio-playback.md)
8. [平台后端冒烟](platform-smoke.md)
9. [XUI 桥接](xui-bridge.md)

## 基础窗口和绘制

- [清屏窗口](clear-window.md)
- [最小窗口和 shape 绘制](minimal-window.md)
- [Shape 绘制](shape-drawing.md)
- [纹理和 Sprite 绘制](texture-sprite.md)
- [文本渲染和 XRF 缓存](text-rendering.md)

## 渲染进阶

- [Sprite Batch](sprite-batch.md)
- [RenderTarget 与离屏绘制](render-target.md)
- [Shader / Material 效果](shader-material.md)
- [Shader 扭曲效果](shader-distortion.md)
- [2.5D 透视 quad](perspective-quad.md)
- [等距视角深度排序](isometric-depth.md)
- [离屏与板卡 EGL 范例](offscreen-egl.md)

## 游戏结构和输入

- [输入事件](input.md)
- [文本输入](text-input.md)
- [多点触控](touch.md)
- [手柄输入](gamepad.md)
- [场景栈](scene-stack.md)
- [APP 模式刷新](app-mode.md)
- [异步资源加载](async-loading.md)
- [音频播放和 3D 声源](audio-playback.md)
- [Debug Overlay](debug-overlay.md)

## 平台

- [平台后端冒烟](platform-smoke.md)
- [Linux X11 窗口](linux-x11.md)
- [Linux EGL 窗口或离屏](linux-egl.md)
- [板卡 Linux EGL](board-linux-egl.md)
- [Android NativeActivity](android-native.md)
- [Android APK 工程](android-apk.md)
- [iOS Simulator](ios-simulator.md)
- [macOS 窗口](macos.md)
- [WebGL2 范例](webgl2.md)
- [小程序 Hello](miniprogram-hello.md)

## XUI

- [XUI 内部孵化](xui-incubation.md)
- [XUI 桥接](xui-bridge.md)
- [XUI 控件范例](xui-controls.md)
- [XUI 表单](xui-form.md)
- [XUI 滚动列表](xui-scroll-list.md)
- [XUI 对话框](xui-dialog.md)

## 阅读建议

- 想先跑通窗口和渲染，从 `minimal-window.md` 开始。
- 想学习资源和纹理，读 `texture-sprite.md`。
- 想做工具、编辑器或 GUI，读 `xui-bridge.md`。
- 想做跨平台验证，读 `platform-smoke.md`。
- 想查函数细节时回 [API 索引](../api/README.md)。
