# Apple 平台接入入门

本教程说明 iOS 和 macOS 的当前兼容策略。

[返回教程索引](README.md) | [Platform API](../api/platform.md) | [平台冒烟范例](../case/platform-smoke.md)

## 优先级

iOS 和 macOS 在 XGE 第一阶段属于低优先级兼容路径。当前策略是保留 OpenGL/OpenGL ES 路径，暂不实现 Metal 后端，但后端接口需要允许未来扩展。

## 图形限制

Apple 平台对 OpenGL 的限制较多，且长期鼓励 Metal。第一版只承诺尽量保持兼容，不把 Apple 平台作为功能设计的最高优先级。

## 接入策略

```text
Sokol Apple backend
  -> OpenGL / OpenGL ES context
  -> XGE platform backend
  -> XGE graphics backend
```

业务代码应避免直接依赖平台 API，把差异留在 platform backend。

## 生命周期

iOS 需要处理前后台切换、surface 尺寸变化、音频焦点和触控。macOS 需要处理窗口 resize、高 DPI 和输入法。

## 常见错误

不要把 Apple 平台限制写进核心 API。核心 API 应保持后端可替换。

不要在第一版为 Metal 设计过度抽象。只需要预留后端接口，不需要提前实现复杂图形抽象层。

不要把 iOS/macOS 未验证能力写成已完成。实测结果应进入维护者验证文档。

## 下一步

- 通用平台策略读 [平台后端与冒烟测试入门](platform-intro.md)。
- Shader 兼容读 [Shader / Material 入门](shader-material-intro.md)。
