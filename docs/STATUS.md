# XGE 当前状态

[返回文档中心](README.md)

## 总体状态

XGE 仍处于快速开发阶段。当前文档描述的是设计目标、已形成的 API 面和开发中的 MVP 能力，不应理解为所有平台均已实机验证完成。

## 已形成闭环的方向

- C API 入口和公开头文件。
- 模块化源码结构与单头文件交付策略。
- OpenGL/GLES/WebGL2 风格图形基线设计。
- Texture、Shape、Sprite、Text、RenderTarget、Material、2.5D API。
- Resource provider、异步加载和 fallback 设计。
- miniaudio 音频封装方向。
- XUI 在 XGE 内孵化的桥接策略。
- 中文正式文档、API 文档、教程和范例结构。

## 需要持续验证的方向

- Linux、macOS、Android、iOS、Web、板卡 Linux 的实机/目标环境验证。
- 小程序宿主桥接。
- 专用 render thread 在不同 context 模型下的行为。
- XUI 从 XGE 内孵化到独立仓库的剥离路径。
- 英文文档生成。

## 平台状态措辞

文档中：

- “可开发验证”表示当前工作站或脚手架可运行相关流程。
- “待验证”表示设计和脚手架存在，但需要目标环境验证。
- “低优先级”表示第一阶段不作为主要推进目标。
- “核心目标”表示第一版必须预留并推进，但仍需实测闭环。

## 查看进度

实现进度看：

```text
dev/docs/XGE_V2_SPEC.md
```

文档工程进度看：

```text
dev/XGE_DOCUMENTATION_SPEC.md
```

平台验证看：

```text
dev/docs/平台后端验证结果.md
```
