# XGE 架构说明

> 状态：中文初稿已生成，待审阅。

XGE V2 采用分层 C API 设计。公开头 `xge.h` 只暴露 C ABI，内部实现按模块拆分到 `src/`，单头文件版本由 `singlehead/` 工具组装。

## 总体分层

```text
应用 / 游戏 / 工具
  -> XGE C API
    -> Core Runtime
    -> Platform Backend
    -> Render Command / Graphics
    -> Resource / Async
    -> Audio
    -> Text / Font
    -> XUI Bridge
      -> xrt / sokol / EGL / miniaudio / stb
```

## Core Runtime

Core 负责：

- `xgeInit` / `xgeUnit`
- game loop 和 manual mode
- frame stats
- scene stack
- timer、FPS、delta
- debug dump 和日志适配

Core 不直接承担所有平台细节。窗口、输入、GL context 和运行态信息由 platform backend 提供。

## Platform Backend

第一版平台策略：

- 桌面、移动和 Web 默认走 Sokol。
- 板卡 Linux 和离屏渲染走 EGL/offscreen。
- 小程序走独立 WebGL2 Canvas bridge。
- Sokol 不是唯一后端，不能把 Sokol API 绑定成 XGE 的唯一平台抽象。

平台能力通过 `xgePlatformCapsGet` 和 `xgePlatformRuntimeGet` 查询。前者报告编译和接口能力，后者报告运行时窗口、framebuffer、DPI 和事件计数。

## Graphics Backend

图形 API 基线：

- Desktop OpenGL 3.3 Core
- OpenGL ES 3.0
- WebGL 2.0

XGE 内部使用 Core Profile 风格的现代渲染路径，但保持 GL Core / GLES3 / WebGL2 的映射能力。默认纹理格式为 RGBA8，默认使用 premultiplied alpha。

## Render Command

公开绘制 API 默认提交到命令队列，帧末 flush。当前实现已经具备基础 command queue、worker drain 和 EGL render thread context owner 路径。完整异步 render thread 仍需要继续把 GPU create/upload/destroy/present 全部迁移到 render queue。

## Resource / Async

资源系统支持：

- 普通文件路径。
- 内存资源。
- `res://` provider。
- xpack provider hook。
- fallback texture/font/sound。
- 异步 image/texture/font/sound 加载。

资源缓存策略暂时交给上层，XGE 只提供加载、引用计数和 fallback 机制。

## Audio

音频后端基于 miniaudio，提供 sound、music、stream、group、fade、3D position 和 listener。公开 API 保持 `xgeSoundPlay`、`xgeMusicPlay`、`xgeStreamPlay` 这样的模块前缀风格。

## Text / Font

文本使用 UTF-8。字体支持 TTF 和 XRF 点阵字体。XRF 定位为快速加载、快速渲染的点阵字体格式，也可作为 TTF 渲染缓存后端。

## XUI Bridge

布局系统归属 XUI，不进入 XGE 内核。XGE 提供事件、时间、资源和绘制能力，XUI 负责 GUI 布局、控件、事件响应和 paint queue。当前 XGE 仓库中仍保留孵化期 XUI 桥接示例，成熟能力会迁移到独立 XUI 仓库。

## 文档边界

- 正式使用文档在 `docs/`。
- 开发设计、历史决策和进度跟踪在 `dev/`。
- API 细节以 `xge.h` 为准。
