# XGE 文档中心

> 面向 XGE 使用者的正式入口。先用 `guide/` 建立使用心智，再用 `api/` 查公开接口，最后用 `case/` 看完整范例。

[项目简介](../README.md)

---

## 快速入口

- [API 索引](api/README.md)
- [教程入口](guide/README.md)
- [范例解析](case/README.md)
- [示例说明](EXAMPLES.md)
- [架构说明](ARCHITECTURE.md)
- [当前状态](STATUS.md)
- [平台与图形兼容性](COMPATIBILITY.md)
- [性能建议](PERFORMANCE.md)
- [最佳实践](BEST_PRACTICES.md)
- [FAQ](FAQ.md)
- [从 XGE V1 迁移](MIGRATION.md)

## 按目标阅读

### 第一次接触 XGE

1. [项目简介](../README.md)
2. [从零开始写第一个 XGE 程序](guide/first-xge-program.md)
3. [最小窗口范例](case/minimal-window.md)
4. [API 索引](api/README.md)

### 准备做 2D 游戏

1. [XGE 架构说明](ARCHITECTURE.md)
2. [Core API](api/core.md)
3. [Graphics API](api/graphics.md)
4. [Scene API](api/scene.md)
5. [Texture / Sprite 范例](case/texture-sprite.md)

### 准备做工具、编辑器或 GUI 应用

1. [运行模式教程](guide/run-mode-intro.md)
2. [RenderTarget 与手动刷新教程](guide/render-target-intro.md)
3. [XUI 桥接范例](case/xui-bridge.md)

### 准备做跨平台发布

1. [平台与图形兼容性](COMPATIBILITY.md)
2. [Platform API](api/platform.md)
3. [平台冒烟范例](case/platform-smoke.md)
4. [离屏与板卡 EGL 范例](case/offscreen-egl.md)

### 从旧版 XGE 或其他 2D 框架迁移

1. [从 XGE V1 迁移到 XGE V2](MIGRATION.md)
2. [XGE 架构说明](ARCHITECTURE.md)
3. [最佳实践](BEST_PRACTICES.md)
4. [性能建议](PERFORMANCE.md)

## 文档分区

- `api/`
  公开类型、函数、生命周期、资源归属、错误码和模块边界。
- `guide/`
  按学习顺序解释什么时候该用什么能力，以及如何从最小程序走到完整集成。
- `case/`
  把初始化、渲染、资源、输入、音频、平台后端和 XUI 串起来的完整范例。
- 顶层主题文档
  架构、状态、兼容性、性能、最佳实践、迁移和 FAQ。

## 语言策略

当前先生成中文正式文档。中文审阅通过后，再为同名文档生成英文 `.en.md` 版本。

## 与 dev 文档的边界

- `docs/`：正式使用文档，面向引擎使用者。
- `dev/docs/`：设计、实现计划、平台验证记录和开发过程文档，面向维护者。

如果你只是使用 XGE，请优先阅读 `docs/`。如果你要参与引擎实现，再阅读 `dev/docs/`。
