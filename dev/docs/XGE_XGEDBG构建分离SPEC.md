# XGE / XGEDBG 构建分离 SPEC

本文档跟踪 `xge` 与 `xgedbg` 的构建分离进度。

关联设计文档：`dev/docs/XGE_XGEDBG构建分离设计.md`

## 进度维护规则

每次开始相关开发前，必须先更新本文档进度：

1. 将即将开发的任务从 `[ ]` 改为 `[~]`。
2. 如果任务范围变化，先调整任务描述或新增子任务。
3. 如果发现阻塞，追加到“阻塞与决策记录”。
4. 开发完成、构建通过、示例或测试通过后，才能改为 `[x]`。
5. 如果任务被放弃或替代，标记为 `[!]` 并写明原因。

状态含义：

- `[ ]` 未开始。
- `[~]` 进行中。
- `[x]` 已完成，并通过必要验证。
- `[!]` 放弃、替代或暂缓，必须说明原因。

完成口径：

- 设计任务：文档更新完成，并明确边界。
- 构建任务：脚本能产生目标产物，并记录命令。
- API 任务：声明、实现、导出符号和示例引用一致。
- 隔离任务：`XGE_DEBUGMODE=0` 编译时不包含对应实现和字段。
- 验证任务：记录文件大小、符号差异或最小运行结果。

## 总体里程碑

- [x] 确认 `XGE_DEBUGMODE` 用于区分调试能力是否进入二进制。
- [x] 确认 `XGE_DEBUGMODE` 不等同于 `XGE_DEBUG` / `XGE_RELEASE`。
- [x] 确认 `xge` 保留运行能力，`xgedbg` 保留完整调试能力。
- [x] 增加 `XGE_DEBUGMODE` 公共宏定义。
- [x] 增加 `xgedbg` 构建产物。
- [x] 完成现有 debug API 与字段盘点。
- [x] 完成第一批调试实现隔离。
- [x] 完成 XUI 调试能力隔离设计落地。
- [x] 完成示例和测试构建分流。
- [x] 完成大小、符号和运行验证。

## 阶段 A：公共宏与产物规则

- [x] 在公共头中定义 `XGE_DEBUGMODE` 默认值。
- [x] 定义内部 helper 宏，例如 `XGE_HAS_DEBUGMODE`。
- [x] 明确 `XGE_DEBUG` / `XGE_RELEASE` 只表达编译配置。
- [x] 文档化四种组合：Release xge、Debug xge、Release xgedbg、Debug xgedbg。
- [x] 明确 Windows 产物命名：`xge.dll/lib` 与 `xgedbg.dll/lib`。
- [x] 明确 Linux/macOS 产物命名。
- [x] 明确单头实现模式如何启用 `XGE_DEBUGMODE=1`。

## 阶段 B：构建脚本

- [x] 增加 `build_dbg_dll.bat`。
- [x] `build_dbg_dll.bat` 输出 `build/xgedbg.dll`。
- [x] `build_dbg_dll.bat` 增加 `-DXGE_DEBUGMODE=1`。
- [x] 确认 `build_dll.bat` 默认输出 `xge.dll` 且 `XGE_DEBUGMODE=0`。
- [x] 评估是否需要 `build_dbg_dll.sh`。
- [x] 增加或调整测试脚本，使普通测试链接 `xge`。
- [x] 增加 debug 测试脚本，使调试测试链接 `xgedbg`。

## 阶段 C：现有调试能力盘点

- [x] 盘点 `xgeDebugGetStats`。
- [x] 盘点 `xgeDebugDumpCaps`。
- [x] 盘点 `xgeFrameStatsGet` 是否属于运行能力或调试能力。
- [x] 盘点 dirty rect 运行机制和 inspector API 的边界。
- [x] 盘点 XUI widget 中调试计数字段。
- [x] 盘点 `src/xge_xui_split_layout.c` 中的 layout debug printf。
- [x] 盘点 GL error polling 和 backend caps dump。
- [x] 盘点 examples 中依赖 debug API 的范例。

## 阶段 D：源码隔离

- [x] 新增或整理 `src/xge_debug.c`。
- [x] 将 backend caps dump 移入 `src/xge_debug.c`。
- [x] 将深度 debug stats 纳入 `XGE_DEBUGMODE` 边界。
- [x] 将 debug printf 和 layout trace 移入 `XGE_DEBUGMODE` 保护块。
- [x] 将 dirty rect 读取 API `xgeDirtyRectCount/Get/Clear` 纳入 debug 边界。
- [x] 保留 dirty rect 正常运行机制在 `xge`。
- [x] 调整聚合实现，只在 `XGE_DEBUGMODE` 打开时包含 debug 实现。
- [x] 确认 `XGE_DEBUGMODE=0` 时调试字符串不进入目标文件。
- [x] 确认 `XGE_DEBUGMODE=0` 时调试 API 调用点不进入编译路径。

## 阶段 E：API 迁移

- [x] 为新调试 API 使用 `#if XGE_DEBUGMODE` 严格声明。
- [x] 新增调试专用 API 允许使用 `xgedbg` 前缀。
- [x] 为已有调试 API 使用 `#if XGE_DEBUGMODE` 严格声明。
- [!] `xge` 中已有 debug API stub 返回 unsupported 或空结果。原因：调试 API 不提供空 stub，避免长期占桩和兼容负担。
- [x] 示例迁移完成后，将已有 debug API 声明纳入 `#if XGE_DEBUGMODE`。
- [x] 仅用于调试的函数调用点必须使用 `#if XGE_DEBUGMODE` 隔离。
- [x] 更新 API 文档，说明调试 API 需要链接 `xgedbg`。
- [x] 增加错误提示，避免用户链接 `xge` 后误用 `xgedbg` API。

## 阶段 F：XUI 调试能力隔离

- [x] XUI layout snapshot API 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] XUI debug overlay 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] Widget inspector 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] Widget id/name/layout/rect tree dump 只进入 `xgedbg`。
- [!] XSON 加载详细 trace 只进入 `xgedbg`。原因：XUI XSON loader 尚未在源码中落地，不能迁移现有实现；约束转入 XUI 声明式 SPEC 后续实现。
- [!] XSON 最小错误码和错误字符串保留在 `xge`。原因：XUI XSON loader 尚未在源码中落地；后续实现时只允许最小错误信息进入 `xge`。
- [!] XUI style cache、theme token、XValue 父表继承保留在 `xge`。原因：相关运行能力尚未在源码中落地；边界决策已确认，具体实现转入 XUI 声明式 SPEC。
- [!] XUI 热重载诊断面板只进入 `xgedbg`。原因：热重载诊断面板尚未实现；后续新增时必须按 `XGE_DEBUGMODE` 隔离。

## 阶段 G：示例与测试分流

- [x] 普通运行示例默认链接 `xge`。
- [x] 全部普通示例构建脚本不再直接编译 `xge.c`，统一链接 `build/xge.lib`。
- [x] debug overlay 示例链接 `xgedbg`。
- [x] layout snapshot 测试链接 `xgedbg`。
- [x] caps dump 示例链接 `xgedbg`。
- [x] CI 或本地验证脚本分别构建 `xge` 和 `xgedbg`。
- [x] 增加一个编译测试，确认 `XGE_DEBUGMODE=0` 下不能调用新调试 API。
- [x] 增加一个编译测试，确认 `XGE_DEBUGMODE=1` 下调试 API 可用。
- [x] 调试示例构建脚本显式增加 `-DXGE_DEBUGMODE=1`。

## 阶段 H：大小、符号与性能验证

- [x] 记录 `xge.dll` 文件大小。
- [x] 记录 `xgedbg.dll` 文件大小。
- [x] 对比导出符号列表。
- [x] 确认 `xge` 不导出新调试 API。
- [x] 基础示例运行 `xge`。
- [x] Debug 示例运行 `xgedbg`。
- [x] 记录基础示例 frame time 或 FPS。
- [x] 确认 XUI layout/paint 热路径在 `xge` 中不访问调试字段。

## 阻塞与决策记录

- [x] 决策：`XGE_DEBUGMODE` 是调试能力开关，不是编译优化级别。
- [x] 决策：`xge` 和 `xgedbg` 可以都使用 Release 优化构建。
- [x] 决策：APP UI 和游戏 UI 的运行能力属于 `xge`。
- [x] 决策：调试 overlay、snapshot、inspector、caps dump 属于 `xgedbg`。
- [x] 决策：调试 API 不在 `xge` 中提供空 stub；声明、实现、调用点必须一起使用宏隔离。
- [x] 决策：dirty rect 读取 API 没有正式业务依赖，可按 inspector API 迁入 `xgedbg`，不背历史兼容负担。
- [x] 决策：新增调试专用 API 可以使用 `xgedbg` 前缀；普通运行 API 继续使用 `xge` 前缀。
- [x] 验证：2026-04-29，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2281931 bytes。
- [x] 验证：2026-04-29，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281931 bytes；当前尚未迁移调试实现，因此大小暂时相同。
- [x] 验证：2026-04-29，第一批严格宏隔离后，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2280325 bytes。
- [x] 验证：2026-04-29，第一批严格宏隔离后，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281931 bytes。
- [x] 验证：2026-04-29，`examples/platform_smoke/build.bat`、`examples/manual_validation/build.bat`、`examples/debug_overlay/build.bat`、`examples/diagnostics_caps_log/build.bat` 构建通过。
- [x] 验证：2026-04-29，`XGE_DEBUGMODE=1` 测试可执行文件 `build/xge_dbg_test.exe` 编译通过。
- [x] 验证：2026-04-29，dirty rect inspector API 迁入 `xgedbg` 后，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2279707 bytes。
- [x] 验证：2026-04-29，dirty rect inspector API 迁入 `xgedbg` 后，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281940 bytes。
- [x] 验证：2026-04-29，`examples/app_mode/build.bat` 和 `examples/xui_tabs_dirty_rect_lab/build.bat` 构建通过。
- [x] 验证：2026-04-29，dirty rect inspector API 迁移后，`XGE_DEBUGMODE=1` 测试可执行文件 `build/xge_dbg_test.exe` 编译通过。
- [x] 验证：2026-04-29，XUI debug counter 和 SplitLayout trace 隔离后，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2278171 bytes。
- [x] 验证：2026-04-29，XUI debug counter 和 SplitLayout trace 隔离后，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281940 bytes。
- [x] 验证：2026-04-29，`examples/xui_split_layout_lab/build.bat` 构建通过。
- [x] 验证：2026-04-29，XUI debug counter 和 SplitLayout trace 隔离后，`XGE_DEBUGMODE=1` 测试可执行文件 `build/xge_dbg_test.exe` 编译通过。
- [x] 验证：2026-04-29，GL error polling 改为随 `XGE_DEBUGMODE` 生效后，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2278171 bytes。
- [x] 验证：2026-04-29，GL error polling 改为随 `XGE_DEBUGMODE` 生效后，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281940 bytes。
- [x] 验证：2026-04-29，`examples/debug_overlay/build.bat` 和 `examples/diagnostics_caps_log/build.bat` 构建通过。
- [x] 验证：2026-04-29，GL error polling 收口后，`XGE_DEBUGMODE=1` 测试可执行文件 `build/xge_dbg_test.exe` 编译通过。
- [x] 验证：2026-04-29，`src/xge_debug.c` 拆分后，`build_dll.bat` 构建通过，`build/xge.dll` 大小 2278171 bytes。
- [x] 验证：2026-04-29，`src/xge_debug.c` 拆分后，`build_dbg_dll.bat` 构建通过，`build/xgedbg.dll` 大小 2281940 bytes。
- [x] 验证：2026-04-29，`src/xge_debug.c` 拆分后，`examples/debug_overlay/build.bat` 和 `examples/diagnostics_caps_log/build.bat` 构建通过。
- [x] 验证：2026-04-29，`src/xge_debug.c` 拆分后，`XGE_DEBUGMODE=1` 测试可执行文件 `build/xge_dbg_test.exe` 编译通过。
- [x] 验证：2026-04-29，`build_dll.bat` 生成 `build/xge.dll` 和 `build/xge.lib`；`build_dbg_dll.bat` 生成 `build/xgedbg.dll` 和 `build/xgedbg.lib`。
- [x] 验证：2026-04-29，`build_test.bat` 已改为链接 `build/xge.lib`，`build_dbg_test.bat` 已新增并链接 `build/xgedbg.lib`；初次分流后两者均完成编译，运行阶段返回 137，已在后续 XUI 修复项中解决。
- [x] 验证：2026-04-29，`objdump -p build/xge.dll` 未发现 `xgeDebug*` 或 `xgedbgDirtyRect*` 导出。
- [x] 验证：2026-04-29，`objdump -p build/xgedbg.dll` 导出 `xgeDebugDumpCaps`、`xgeDebugGetStats`、`xgedbgDirtyRectClear`、`xgedbgDirtyRectCount`、`xgedbgDirtyRectGet`。
- [x] 验证：2026-04-29，`build_debug_api_compile_test.bat` 通过；`XGE_DEBUGMODE=0` 误用调试 API 按预期编译失败，`XGE_DEBUGMODE=1` 调试 API 编译链接通过。
- [x] 验证：2026-04-29，`examples/app_mode/build.bat` 改为链接 `build/xge.lib` 后构建通过。
- [x] 验证：2026-04-29，`examples/debug_overlay/build.bat`、`examples/diagnostics_caps_log/build.bat`、`examples/xui_tabs_dirty_rect_lab/build.bat` 改为链接 `build/xgedbg.lib` 后构建通过。
- [x] 验证：2026-04-29，`build/xge.dll` 未包含 `xgeDebugDumpCaps`、`xgeDebugGetStats`、`xgedbgDirtyRect`、`OpenGL Vendor:`、`Sokol Target:` 等明显调试字符串；`build/xgedbg.dll` 包含对应调试 API 与 caps dump 字符串。
- [x] 验证：2026-04-29，全部普通示例构建脚本已不再直接编译 `xge.c`，统一改为链接 `build/xge.lib`；抽样验证 `examples/async/build.bat`、`examples/platform_smoke/build.bat`、`examples/xui_layout_gallery/build.bat`、`examples/manual_validation/build.bat` 构建通过。
- [x] 验证：2026-04-29，新增 `build_verify_xge_split.bat`，已通过；覆盖 `xge.dll`、`xgedbg.dll`、调试 API 编译边界、普通示例和调试示例构建。
- [x] 验证：2026-04-29，`build/xge_core_lifecycle.exe --frames 1` 运行通过，输出 `fps=60`、`frame_ms=0.000`、`avg_ms=0.000`、`max_ms=0.000`。
- [x] 验证：2026-04-29，`build/xge_platform_smoke.exe --frames 1` 运行通过，确认基础示例可通过 `xge.dll` 加载并退出。
- [x] 验证：2026-04-29，`build/xge_diagnostics_caps_log.exe --frames 1` 运行通过，确认调试示例可通过 `xgedbg.dll` 输出 caps dump 和 debug stats。
- [x] 验证：2026-04-29，`gcc -E -DXGE_IMPLEMENTATION -DXGE_DEBUGMODE=0 -I. xge.c` 的预处理输出未包含 `iDirtyLayoutCount`、`iDirtyPaintCount`、`__xgeXuiSplitLayoutDebugSizes`、SplitLayout trace、`xgeDebugDumpCaps` 或 `xgedbgDirtyRect`。
- [x] 验证：2026-04-29，`build_debug_api_compile_test.bat` 增加误链接测试；`XGE_DEBUGMODE=1` 调用方链接 `build/xge.lib` 按预期失败，链接 `build/xgedbg.lib` 成功。
- [x] 验证：2026-04-29，新增 `xgedbgXuiWidgetTreeDump` 与 `xgedbgXuiLayoutSnapshot`；`objdump -p build/xge.dll` 未导出新 XUI 调试 API，`objdump -p build/xgedbg.dll` 已导出。
- [x] 验证：2026-04-29，`build_debug_api_compile_test.bat` 编译并运行 `test/debug_xui_snapshot.c`，确认 XUI snapshot API 可通过 `xgedbg.lib` 使用，输出 `xui snapshot bytes=683`。
- [x] 验证：2026-04-29，`build_verify_xge_split.bat` 已重新通过，覆盖新增 XUI snapshot 调试 API。
- [x] 修复：2026-04-29，XUI widget 默认不再全部启用 clip，root/overlay/普通 widget 避免无意义 clip set/clear；ScrollView、ListView、Window、Popup 等需要裁剪的控件在 Init 中显式开启。
- [x] 修复：2026-04-29，`xgeXuiHitTest` 返回视觉命中的最上层可见 widget，事件分发继续沿父链寻找处理者。
- [x] 修复：2026-04-29，`xgeXuiTextInsert` 插入到中间时不再用会写 `NUL` 的 normalized copy 覆盖尾部文本。
- [x] 验证：2026-04-29，`build_test.bat` 已通过，普通测试链接 `build/xge.lib` 并运行 `build/xge_test.exe` 成功。
- [x] 验证：2026-04-29，`build_dbg_test.bat` 已通过，debug 测试链接 `build/xgedbg.lib` 并运行 `build/xge_dbg_test.exe` 成功。
- [x] 验证：2026-04-29，修复已知测试失败后，`build_verify_xge_split.bat` 已重新通过。
- [x] 验证：2026-04-29，新增 `xgedbgXuiWidgetInspect`、`xgedbgXuiWidgetInspectAt` 与 `xgedbgXuiDebugOverlayPaint`；`build_debug_api_compile_test.bat` 已通过，确认 `XGE_DEBUGMODE=0` 不可调用、误链接 `xge.lib` 失败、链接 `xgedbg.lib` 成功。
- [x] 验证：2026-04-29，`objdump -p build/xge.dll` 未导出新增 XUI overlay/inspector API；`objdump -p build/xgedbg.dll` 已导出 `xgedbgXuiDebugOverlayPaint`、`xgedbgXuiWidgetInspect`、`xgedbgXuiWidgetInspectAt`。
- [x] 验证：2026-04-29，新增 XUI overlay/inspector API 后，`build_verify_xge_split.bat`、`build_test.bat`、`build_dbg_test.bat` 均通过。
- [x] 决策：2026-04-29，XUI XSON loader、style cache、theme token、XValue 父表继承和热重载诊断面板尚未在源码中落地，不为构建分离新增空 stub；后续实现必须分别遵守 `xge`/`xgedbg` 边界。
- [x] 验证：2026-04-29，新增 `build_dbg_dll.sh` 输出 `build/libxgedbg.so` 且显式使用 `-DXGE_DEBUGMODE=1`；`build_dll.sh` 显式使用 `-DXGE_DEBUGMODE=0`。
- [!] 验证：2026-04-29，当前 Windows 环境未提供 `sh`，无法本地执行 `.sh` 构建脚本；已完成脚本文本规则检查，实际 Linux/macOS 构建需在对应环境验证。
- [!] 待确认：已有 debug API 第一阶段是否保留兼容 stub。结论：不保留，改为严格宏隔离。
- [x] 待确认：dirty rect 读取 API 是否已有正式业务依赖。结论：没有，不背历史包袱。
- [x] 待确认：动态库导出名是否需要保持 `xge` 前缀还是新增 `xgedbg` 前缀。结论：新增调试专用 API 可使用 `xgedbg` 前缀。

## 当前开发指针

下一次开发前请先更新本节：

- 当前任务：无。
- 当前状态：`xge` / `xgedbg` 第一阶段构建分离已完成；XUI XSON loader、style cache、theme token、XValue 父表继承和热重载诊断面板转入 XUI 声明式 SPEC 后续实现。
- 最近更新时间：2026-04-29。
