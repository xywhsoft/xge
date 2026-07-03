# Android 接入入门

本教程说明 XGE 在 Android 上的接入策略和第一版注意事项。

[返回教程索引](README.md) | [Platform API](../api/platform.md) | [平台冒烟范例](../case/platform-smoke.md)

## 图形基线

Android 使用 OpenGL ES 3.0 作为图形基线。目标现代设备时，GLES3 能提供更完整的纹理、FBO、shader 和 buffer 能力。

## 容器后端

第一版优先使用 Sokol/NativeActivity 路径跑通窗口、触控、生命周期和音频。APK 工程可以后续作为分发脚手架完善。

```text
NativeActivity / app glue
  -> Sokol backend
  -> GLES3 context
  -> XGE runtime
```

## 触控和高 DPI

Android 输入优先走 touch API：

```c
int count = xgeTouchGetCount();
```

逻辑尺寸和 framebuffer 尺寸可能不同，应通过 `xgePlatformRuntimeGet` 查询 DPI scale。

## 生命周期

Android 会频繁触发 pause/resume、surface recreate 和音频焦点变化。业务层应准备：

- 暂停 update。
- 暂停或降低音频。
- surface 重建后恢复 GPU 资源。

## 常见错误

不要假设 Android 资源能用桌面相对路径直接打开。APK asset、外部目录和 provider 需要明确接入。

不要在非 GL context 线程创建纹理。异步加载只做 IO/解码，上传排队给渲染线程。

不要忽略 pause/resume。后台继续高频渲染会浪费电量，也可能被系统杀掉。

## 下一步

- 渲染线程读 [专用 Render Thread 入门](render-thread-intro.md)。
- 输入读 [输入事件入门](input-intro.md)。
