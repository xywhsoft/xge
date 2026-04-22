# XGE

XGE 是一个以跨平台、轻量和现代 2D 渲染为目标的 C 语言游戏引擎。它面向需要在桌面、移动端、Web、小程序、板卡 Linux 和离屏环境中复用同一套 2D 引擎能力的开发者。

XGE 的目标不是复刻旧版 XGE 1.x 的 DirectDraw 时代接口，而是在保留功能覆盖面的前提下，用现代 GPU、现代输入、异步资源、音频、字体、RenderTarget、2.5D 和 GUI 桥接能力重建一个更适合现代游戏和工具开发的底层引擎。

## 核心能力

| 能力 | 说明 |
| --- | --- |
| Core Runtime | 初始化、运行循环、手动刷新、场景栈、时间和调试统计。 |
| Platform Backend | Sokol 窗口后端、EGL/offscreen 后端、小程序桥接和平台能力报告。 |
| Graphics Backend | OpenGL 3.3 Core、OpenGL ES 3.0、WebGL2 映射和 GL 函数加载。 |
| 2D Rendering | Texture、Sprite、Shape、Text、SpriteBatch、RenderTarget 和 RenderPass。 |
| 2.5D | 自定义顶点、mesh、透视 quad、depth test 和轻量 3D 顶点 API。 |
| Resource | 文件、内存、`res://` provider、xpack provider hook 和 fallback resource。 |
| Async | 异步 image/texture/font/sound 加载和 GPU upload queue。 |
| Audio | sound、music、stream、group、loop、fade、3D position 和 listener。 |
| Text / Font | UTF-8、TTF、XRF 点阵字体、中文 UCS2 范围和 fallback font。 |
| XUI Bridge | XGE 内部孵化的 XUI 桥接与后续独立 XUI 仓库协作。 |

## 文档入口

正式文档从 [docs/README.md](docs/README.md) 开始。

建议阅读顺序：

1. [从零开始写第一个 XGE 程序](docs/guide/first-xge-program.md)
2. [XGE 架构说明](docs/ARCHITECTURE.md)
3. [API 索引](docs/api/README.md)
4. [范例解析](docs/case/README.md)
5. [示例说明](docs/EXAMPLES.md)
6. [当前状态](docs/STATUS.md)
7. [平台与图形兼容性](docs/COMPATIBILITY.md)
8. [性能建议](docs/PERFORMANCE.md)
9. [最佳实践](docs/BEST_PRACTICES.md)
10. [FAQ](docs/FAQ.md)

开发设计、历史规划和进度跟踪文档位于 `dev/`。正式使用文档位于 `docs/`，中文先行，英文版将在中文审阅后补齐为 `.en.md` 文件。

## 快速开始

### 环境要求

当前 Windows 本地开发路径需要：

- Windows
- MinGW-w64 `gcc` 位于 `PATH`
- PowerShell 或 cmd

跨平台目标包括 Windows、Linux、macOS、Android、iOS、Web/Emscripten、小程序、板卡 Linux EGL 和离屏渲染。未完成实机验证的平台不会在文档中标记为“已验证”。

### 构建最小示例

```bat
build_exe.bat
```

### 运行核心测试

```bat
build_test.bat
```

### 平台冒烟

```bat
check_platform_smoke.bat
```

该脚本会构建 platform smoke 示例，自动运行若干帧，输出 caps/runtime 日志并检查日志完整性。

## 目录结构

| 路径 | 说明 |
| --- | --- |
| `xge.h` | 公开 C API。 |
| `xge.c` | 聚合编译入口。 |
| `src/` | 按模块拆分的引擎实现。 |
| `examples/` | 可运行示例。 |
| `test/` | 纯 C 回归测试。 |
| `platform/` | Android、iOS、Web 等平台 scaffold。 |
| `lib/` | 第三方依赖，包括 xrt、sokol、stb、miniaudio。 |
| `singlehead/` | 单头文件生成器和单头版本输出。 |
| `docs/` | 面向使用者的正式文档。 |
| `dev/` | 设计文档、进度 spec 和历史开发资料。 |

## 当前状态

XGE 仍处于快速开发阶段。Windows 本地构建、核心 API、渲染、资源、音频、字体、XUI 孵化和平台脚手架已经形成基础闭环；Linux/macOS/Android/iOS/Web/板卡 EGL 等平台仍需要目标环境验证后才能关闭对应平台任务。
