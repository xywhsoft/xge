# XGE

XGE 是一个 C 语言图形与应用界面库。它提供窗口与渲染循环、2D 图形和文本、资源与输入，以及建立在 XGE 上的保留式 XUI 控件库。

当前公开版本为 **3.0.0**。[`include/xge.h`](include/xge.h) 和 [`include/xui.h`](include/xui.h) 是公开 API 权威；本文档和范例说明如何使用这些 API，不承诺未在头文件中声明的内部实现细节。

图片方向与升级指南： [XGE 3 coordinate contract](docs/guide/TEXTURE_COORDINATES.md).
## 快速入口

- [构建与验证](docs/guide/BUILD.md)：Windows 构建、测试与输出位置。
- [XGE 使用指南](docs/guide/XGE.md)：运行时、绘制、资源、输入与生命周期。
- [2D 粒子系统](docs/guide/PARTICLES.md)：独立 CPU 模拟、配置、批渲染与七个经典场景范例。
- [通用 3D 基础增强](docs/guide/FOUNDATION.md)：增量场景、空间索引、分步装配与回收、共享蒙皮、动画混合和 SDK 验收。
- [XUI 使用指南](docs/guide/XUI.md)：控件、布局、事件和范例选择。
- [范例索引](docs/guide/EXAMPLES.md)：按功能选择可运行范例。
- [平台说明](docs/guide/PLATFORM.md)：Windows、Android、iOS 和 Web 的当前入口。
- [Emoji 支持](docs/guide/EMOJI.md)：内置 Emoji 的运行时行为与自定义包。

英文概览见 [README.en.md](README.en.md)。

## 仓库结构

| 路径 | 用途 |
| --- | --- |
| `include/` | XGE、XUI 和 Document 公开 C API。 |
| `single/xge.c` | 单文件 XGE 实现入口。 |
| `src/xge/`、`src/xui/` | 按子系统组织的私有实现。应用不应依赖其中的私有类型或函数。 |
| `modules/` | 项目自有的独立模块，例如 xLayout。 |
| `extlibs/` | 固定版本的第三方依赖。 |
| `examples/xge/`、`examples/xui/` | 可运行的 XGE/XUI 使用示例。 |
| `tests/xge/`、`tests/xui/` | 自动化测试。 |
| `apps/` | 基于 XGE/XUI 的完整工具和应用。 |
| `platform/` | Android、iOS、Web 的构建入口和平台适配。 |
| `res/xge/`、`res/xui/` | 运行时内置资源。 |
| `tools/build/`、`tools/test/` | 统一构建与测试入口。 |
| `tools/check/`、`tools/generate/`、`tools/package/` | 检查、资源生成和发布工具。 |
| `config/` | 构建配置和源码清单。 |

`build/` 与 `artifacts/` 是本地生成目录，不属于源代码发布内容。

## 发布边界

- 应用只包含和链接公开头文件、实现源码或由本仓库构建得到的库文件。
- 通过 `xge*`、`xui*` 公开函数创建和释放对象；不要访问 `src/` 中的结构体定义，也不要保存私有指针。
- `include/xge.h` 中以不完整类型暴露的资源句柄按其 API 约定使用。资源对象的内部布局不构成 ABI。
- 平台目录中的内容是各平台的接入起点；是否可用于交付取决于在目标设备或浏览器上完成构建与测试。

## 最小验证

在 Windows / MinGW-w64 环境中：

```bat
build.bat
tools\test\build_test.bat
tools\test\build_dbg_test.bat
tests\xui\build_context_test.bat
tests\xui\build_widget_test.bat
```

更多命令和前置条件见 [构建与验证](docs/guide/BUILD.md)。
