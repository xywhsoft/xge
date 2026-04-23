# XGE 范例解析

> 面向“多个模块如何串成一个完整程序”的案例入口。这里与 `guide/` 的区别是：`guide/` 先建立使用心智，`case/` 直接展示完整问题的解决方案。

[返回文档中心](../README.md)

---

## 主线必读

第一次系统阅读范例时，建议先按下面顺序走：

1. [最小窗口和 shape 绘制](minimal-window.md)
2. [Core 生命周期](core-lifecycle.md)
3. [诊断、Caps 与日志](diagnostics-caps-log.md)
4. [Viewport、Clip 与 Camera](viewport-clip-camera.md)
5. [Image 与 Texture Lab](image-texture-lab.md)
6. [Font Text Lab](font-text-lab.md)
7. [Resource Provider Lab](resource-provider-lab.md)
8. [Input State Lab](input-state-lab.md)
9. [Audio Lab](audio-lab.md)
10. [Scene Lifecycle Lab](scene-lifecycle-lab.md)
11. [Render Target Lab](render-target-lab.md)
12. [Shader Variant Lab](shader-variant-lab.md)
13. [Material Mesh Buffer Lab](material-mesh-buffer-lab.md)
14. [Blend Depth Color Lab](blend-depth-color-lab.md)
15. [Shape Full Gallery](shape-full-gallery.md)
16. [Async Assets Lab](async-assets-lab.md)
17. [Platform Runtime Lab](platform-runtime-lab.md)
18. [Offscreen EGL Lab](offscreen-egl-lab.md)
18. [纹理和 Sprite 绘制](texture-sprite.md)
19. [场景栈](scene-stack.md)
20. [RenderTarget 与离屏绘制](render-target.md)
21. [Shader / Material 效果](shader-material.md)
22. [2.5D 透视 quad](perspective-quad.md)
23. [音频播放和 3D 声源](audio-playback.md)
24. [平台后端冒烟](platform-smoke.md)
25. [XUI 桥接](xui-bridge.md)

## 基础窗口和绘制

- [清屏窗口](clear-window.md)
- [Core 生命周期](core-lifecycle.md)
- [诊断、Caps 与日志](diagnostics-caps-log.md)
- [Viewport、Clip 与 Camera](viewport-clip-camera.md)
- [Image 与 Texture Lab](image-texture-lab.md)
- [Font Text Lab](font-text-lab.md)
- [Resource Provider Lab](resource-provider-lab.md)
- [最小窗口和 shape 绘制](minimal-window.md)
- [Shape 绘制](shape-drawing.md)
- [纹理和 Sprite 绘制](texture-sprite.md)
- [文本渲染和 XRF 缓存](text-rendering.md)

## 渲染进阶

- [Sprite Batch](sprite-batch.md)
- [Render Target Lab](render-target-lab.md)
- [RenderTarget 与离屏绘制](render-target.md)
- [Shader Variant Lab](shader-variant-lab.md)
- [Material Mesh Buffer Lab](material-mesh-buffer-lab.md)
- [Blend Depth Color Lab](blend-depth-color-lab.md)
- [Shape Full Gallery](shape-full-gallery.md)
- [Async Assets Lab](async-assets-lab.md)
- [Platform Runtime Lab](platform-runtime-lab.md)
- [Shader / Material 效果](shader-material.md)
- [Shader 扭曲效果](shader-distortion.md)
- [2.5D 透视 quad](perspective-quad.md)
- [等距视角深度排序](isometric-depth.md)
- [离屏与板卡 EGL 范例](offscreen-egl.md)

## 游戏结构和输入

- [输入事件](input.md)
- [Input State Lab](input-state-lab.md)
- [文本输入](text-input.md)
- [多点触控](touch.md)
- [手柄输入](gamepad.md)
- [场景栈](scene-stack.md)
- [Scene Lifecycle Lab](scene-lifecycle-lab.md)
- [APP 模式刷新](app-mode.md)
- [异步资源加载](async-loading.md)
- [音频播放和 3D 声源](audio-playback.md)
- [Audio Lab](audio-lab.md)
- [Debug Overlay](debug-overlay.md)

## 平台

- [Platform Runtime Lab](platform-runtime-lab.md)
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
