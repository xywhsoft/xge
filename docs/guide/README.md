# XGE 教程文档

> 面向第一次系统学习 XGE 的读者。这里回答“什么时候该用什么能力、为什么这样组合、最小调用顺序是什么”。

[返回文档中心](../README.md)

---

## 建议起步路线

| 顺序 | 教程 | 难度 | 前置 |
| --- | --- | --- | --- |
| 1 | [从零开始写第一个 XGE 程序](first-xge-program.md) | 入门 | 无 |
| 2 | [运行模式入门](run-mode-intro.md) | 入门 | 第一个程序 |
| 3 | [项目结构入门](project-structure.md) | 入门 | 第一个程序 |
| 4 | [资源加载入门](resource-intro.md) | 入门 | 项目结构 |
| 5 | [资源与纹理入门](texture-intro.md) | 入门 | 资源加载 |
| 6 | [Shape、Sprite 与文本绘制入门](drawing-intro.md) | 入门 | 纹理 |
| 7 | [坐标、Camera、Viewport 与 Clip 入门](coordinate-intro.md) | 基础 | 绘制 |
| 8 | [场景系统入门](scene-intro.md) | 基础 | 运行模式 |
| 9 | [输入事件入门](input-intro.md) | 基础 | 场景 |
| 10 | [音频入门](audio-intro.md) | 基础 | 资源加载 |
| 11 | [平台后端与冒烟测试入门](platform-intro.md) | 进阶 | 第一个程序 |
| 12 | [XUI 桥接入门](xui-bridge-intro.md) | 进阶 | 绘制、输入 |
| 13 | [范例解析](../case/README.md) | 综合 | 前面教程 |

## 按主题阅读

### 基础窗口和循环

| 教程 | 难度 | 前置 |
| --- | --- | --- |
| [从零开始写第一个 XGE 程序](first-xge-program.md) | 入门 | 无 |
| [运行模式入门](run-mode-intro.md) | 入门 | 第一个程序 |
| [项目结构入门](project-structure.md) | 入门 | 无 |
| [错误、日志与调试信息](error-and-log.md) | 基础 | 第一个程序 |
| [场景系统入门](scene-intro.md) | 基础 | 运行模式 |

### 2D 渲染

| 教程 | 难度 | 前置 |
| --- | --- | --- |
| [资源加载入门](resource-intro.md) | 入门 | 项目结构 |
| [资源与纹理入门](texture-intro.md) | 入门 | 资源加载 |
| [Shape、Sprite 与文本绘制入门](drawing-intro.md) | 入门 | 纹理 |
| [坐标、Camera、Viewport 与 Clip 入门](coordinate-intro.md) | 基础 | 绘制 |
| [Premultiplied Alpha 与 Blend 入门](blend-alpha-intro.md) | 基础 | 绘制 |
| [RenderTarget 与离屏渲染入门](render-target-intro.md) | 进阶 | 绘制、纹理 |
| [Shader / Material 入门](shader-material-intro.md) | 进阶 | 绘制、纹理 |
| [2.5D 入门](2.5d-intro.md) | 进阶 | 坐标、shader |

### 游戏结构和资源

| 教程 | 难度 | 前置 |
| --- | --- | --- |
| [输入事件入门](input-intro.md) | 基础 | 运行模式 |
| [异步资源加载入门](async-intro.md) | 进阶 | 资源加载 |
| [音频入门](audio-intro.md) | 基础 | 资源加载 |

### 跨平台和工具应用

| 教程 | 难度 | 前置 |
| --- | --- | --- |
| [平台后端与冒烟测试入门](platform-intro.md) | 进阶 | 第一个程序 |
| [专用 Render Thread 入门](render-thread-intro.md) | 高级 | 平台、异步 |
| [离屏渲染入门](offscreen-intro.md) | 高级 | RenderTarget、平台 |
| [小程序接入入门](miniprogram-intro.md) | 高级 | 平台、输入 |
| [WebGL2 / Emscripten 入门](web-intro.md) | 高级 | 平台、资源 |
| [Android 接入入门](android-intro.md) | 高级 | 平台、输入 |
| [Apple 平台接入入门](ios-macos-intro.md) | 高级 | 平台 |
| [板卡 Linux 入门](board-linux-intro.md) | 高级 | EGL、render thread |

### XUI

| 教程 | 难度 | 前置 |
| --- | --- | --- |
| [XUI 桥接入门](xui-bridge-intro.md) | 进阶 | 绘制、输入 |
| [XUI 布局入门](xui-layout-intro.md) | 进阶 | XUI 桥接 |
| [XUI 控件入门](xui-controls-intro.md) | 进阶 | XUI 布局 |
| [XUI 文本输入与 IME 入门](xui-input-ime-intro.md) | 高级 | XUI 控件、输入 |
| [XUI 渲染与刷新入门](xui-render-intro.md) | 高级 | XUI 桥接、APP 模式 |

## 什么时候看 API、什么时候看范例

- 已经知道要调用哪个模块，只想查函数和资源释放规则，去 [API 索引](../api/README.md)。
- 想看一个完整程序如何把多个模块串起来，去 [范例解析](../case/README.md)。
