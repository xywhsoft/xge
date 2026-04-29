# XGE / XGEDBG 构建分离设计

本文档定义 `xge` 与 `xgedbg` 的构建边界。目标是在不削弱引擎调试能力的前提下，让正式运行库保持更小体积和更低热路径成本。

## 核心目标

- `xge` 是默认运行库，只包含游戏和 APP 正常运行所需能力。
- `xgedbg` 是带调试能力的运行库，包含诊断、检查、可视化、快照、统计和开发工具 API。
- 使用全局宏 `XGE_DEBUGMODE` 区分是否编译调试能力。
- `XGE_DEBUGMODE` 不等同于 `XGE_DEBUG`。
- 调试能力可以完整保留在 `xgedbg` 中，不因为 `xge` 的体积和效率目标被删减。
- `xge` 的热路径不为调试工具保留额外分支、字符串、统计字段或导出符号。

## 宏定义分层

现有 `XGE_DEBUG` / `XGE_RELEASE` 适合表达编译配置，例如是否启用断言、低层一致性检查、额外错误日志。

新增 `XGE_DEBUGMODE` 表达产品形态：

```c
#if !defined(XGE_DEBUGMODE)
#define XGE_DEBUGMODE 0
#endif
```

建议语义：

- `XGE_DEBUGMODE == 0`：构建 `xge`，不包含引擎调试工具。
- `XGE_DEBUGMODE != 0`：构建 `xgedbg`，包含引擎调试工具。

组合关系：

| 编译配置 | XGE_DEBUGMODE | 产物 | 含义 |
| --- | --- | --- | --- |
| Release | 0 | `xge` | 正式运行库 |
| Debug | 0 | `xge` | 用于开发时检查 release 形态热路径 |
| Release | 1 | `xgedbg` | 推荐的调试工具运行库，保留优化 |
| Debug | 1 | `xgedbg` | 深度排查用，体积和速度不是优先目标 |

## 产物命名

Windows 动态库：

- `build/xge.dll`
- `build/xge.lib`
- `build/xgedbg.dll`
- `build/xgedbg.lib`

Linux/macOS 动态库：

- `build/libxge.so` 或 `build/libxge.dylib`
- `build/libxgedbg.so` 或 `build/libxgedbg.dylib`

单头/单实现模式：

- 默认 `#include "xge.h"` + `XGE_IMPLEMENTATION` 构建 `xge`。
- 显式 `-DXGE_DEBUGMODE=1` 构建 `xgedbg`。
- 可增加 `xgedbg.c` 包装入口，内部定义 `XGE_DEBUGMODE=1` 后包含实现。

## 能力边界

必须保留在 `xge`：

- 正常渲染、输入、资源、窗口、字体、图像、XUI retained tree。
- APP UI 所需的脏区机制本身，如果它参与正常绘制优化。
- 最小错误返回、必要日志和崩溃前诊断。
- 对运行正确性有必要的轻量断言，仍由 `XGE_DEBUG` 控制。

必须进入 `xgedbg` 或由 `XGE_DEBUGMODE` 保护：

- Debug overlay。
- Layout snapshot 和 UI tree dump。
- XUI widget id/type/rect/margin/padding/dirty/hit/focus 可视化。
- GL caps dump、backend caps dump、资源统计 dump。
- 每帧深度统计、历史曲线、计数器字符串名。
- `glGetError` 轮询、GPU/renderer 验证路径。
- dirty rect inspector API，不包括 dirty rect 运行机制本身。
- 开发期 printf、临时 layout trace、控件树遍历 dump。
- 热重载诊断面板和 XSON 加载详细解释器。

需要逐项判断：

- `xgeDebugGetStats`：只在 `XGE_DEBUGMODE` 下声明、实现和调用。
- `xgeDebugDumpCaps`：只在 `XGE_DEBUGMODE` 下声明、实现和调用。
- Dirty rect APIs：`xgeInvalidateRect` 属运行功能；`xgeDirtyRectCount/Get/Clear` 没有正式业务依赖，按 inspector API 迁入 `xgedbg`，不背历史兼容负担。
- XUI 内部 dirty counter：如果只用于调试显示，应放入 `XGE_DEBUGMODE` 字段块。

调试 API 命名：

- 已有 `xgeDebug*` API 可以在迁移期继续使用，但只在 `XGE_DEBUGMODE` 下声明、实现和调用。
- 新增调试专用 API 允许使用 `xgedbg` 前缀，例如 `xgedbgDirtyRectGet`、`xgedbgXuiSnapshotWrite`。
- `xgedbg` 前缀 API 不进入 `xge`，不提供同名空 stub。
- 普通运行 API 继续使用 `xge` 前缀。

## API 暴露策略

新调试 API 使用严格策略：

```c
#if XGE_DEBUGMODE
XGE_API void xgeDebugDumpCaps(void);
XGE_API int xgeXuiDebugSnapshot(...);
#endif
```

这样 `xge` 用户无法意外依赖调试 API，也不会为调试 API 保留导出符号。

使用调试 API 的工程必须同时满足：

- 编译调用方时显式定义 `XGE_DEBUGMODE=1`。
- 链接 `xgedbg` 产物，例如 Windows 下链接 `build/xgedbg.lib` 并部署 `build/xgedbg.dll`。
- 不要在同一目标中混用 `XGE_DEBUGMODE=1` 的头文件声明和 `xge` 链接库；这种误用应表现为链接失败，而不是由 `xge` 提供空实现。
- 只需要正常运行 UI、渲染、输入、资源和 dirty rect 失效机制时，继续使用 `XGE_DEBUGMODE=0` 与 `xge`。

已有调试 API 也使用严格迁移策略：

1. 声明纳入 `#if XGE_DEBUGMODE`。
2. 实现纳入 `#if XGE_DEBUGMODE`，或移动到只由 `xgedbg` 聚合的源码文件。
3. 调用点纳入 `#if XGE_DEBUGMODE`，或让对应示例、工具、测试显式使用 `-DXGE_DEBUGMODE=1`。
4. 不在 `xge` 中提供空 stub、兼容占桩函数或无意义返回值。

函数调用也必须被隔离。只要某个调用只服务于调试能力，即使被调用函数本身未来会被移入 `xgedbg`，调用点也不能裸露在 `xge` 编译路径中。

## 源码组织

建议按职责拆分：

```text
src/xge_core.c              常规运行能力
src/xge_impl.c              聚合实现
src/xge_debug.c             引擎调试 API
src/xge_xui_debug.c         XUI 调试、快照、overlay
src/xge_xui_layout.c        正常布局
```

聚合文件只在 `XGE_DEBUGMODE` 打开时包含调试实现：

```c
#include "src/xge_core.c"

#if XGE_DEBUGMODE
#include "src/xge_debug.c"
#include "src/xge_xui_debug.c"
#endif
```

结构体中调试字段也必须被保护：

```c
typedef struct xge_xui_widget_t {
	...
#if XGE_DEBUGMODE
	int iDirtyLayoutCount;
	int iDirtyPaintCount;
#endif
} xge_xui_widget_t;
```

## XUI 关联要求

声明式 XUI 需要强调开发体验，但这些工具不应污染 `xge`：

- XSON 加载错误的最小错误码和错误字符串可以保留在 `xge`。
- 详细字段路径、源位置、修复建议、trace 应进入 `xgedbg`。
- Layout snapshot、可视化 overlay、widget inspector 只进入 `xgedbg`。
- XSON 热重载建议只进入 `xgedbg`，正式运行库只保留显式 reload API。
- XUI 样式 cache、XValue 父表链是运行功能，必须保留在 `xge`。

## 构建脚本策略

建议增加独立构建入口：

```text
build_dll.bat              -> xge.dll
build_dbg_dll.bat          -> xgedbg.dll, -DXGE_DEBUGMODE=1
build_test.bat             -> 默认链接 xge
build_dbg_test.bat         -> 链接 xgedbg
```

也可以将脚本参数化：

```text
build_dll.bat xge
build_dll.bat xgedbg
```

第一阶段优先增加独立脚本，避免破坏现有调用习惯。

## 验证指标

每次推进隔离后，需要记录：

- `xge.dll` 和 `xgedbg.dll` 文件大小。
- `xge.lib` 和 `xgedbg.lib` 导出符号差异。
- 基础示例 FPS 或 frame time 对比。
- XUI layout/paint 热路径是否仍访问调试字段。
- 使用 `XGE_DEBUGMODE=0` 编译时，调试实现文件是否未参与编译。
- 使用 `XGE_DEBUGMODE=0` 编译时，调试 API 调用点是否也未参与编译。

## 迁移顺序

1. 增加 `XGE_DEBUGMODE` 宏和产物命名规范。
2. 建立 `xgedbg` 构建脚本。
3. 盘点现有 debug API、debug 字段和 debug printf。
4. 将明显调试代码移入 `src/xge_debug.c`。
5. 将 XUI debug overlay/snapshot 设计为 `xgedbg` 专属。
6. 调整示例：运行示例链接 `xge`，调试示例链接 `xgedbg`。
7. 增加大小、符号和最小运行验证。

## 关键原则

- `XGE_DEBUG` 管编译检查，`XGE_DEBUGMODE` 管调试能力是否存在。
- `xge` 不为调试工具牺牲运行效率和程序大小。
- `xgedbg` 不因为正式库目标裁剪调试体验。
- APP UI 和游戏 UI 的运行能力属于 `xge`，开发诊断能力属于 `xgedbg`。
- 不为调试 API 在 `xge` 中保留空 stub；误用应在编译期或链接期暴露。
