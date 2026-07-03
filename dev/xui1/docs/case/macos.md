# macOS 窗口

本案例说明 macOS 桌面窗口路径的验证重点。

[返回范例解析](README.md) | [Apple 平台教程](../guide/ios-macos-intro.md) | [Platform API](../api/platform.md)

## 状态

macOS 是低优先级兼容目标。当前策略是保留 OpenGL 路径，并为未来 Metal 后端预留接口。

## 目标

```text
Sokol macOS backend
  -> OpenGL context
  -> XGE runtime
```

## 验证重点

- 窗口创建、resize、close。
- 高 DPI framebuffer 尺寸。
- OpenGL context 和 shader 编译。
- 键盘、鼠标、文本输入。
- 音频初始化。

## 常见失败原因

系统 OpenGL 限制导致 shader 版本不匹配。

Retina 下逻辑尺寸和 framebuffer 尺寸混用。

应用 bundle 配置不完整。
