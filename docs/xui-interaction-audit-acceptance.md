# XUI 交互审计修复验收报告

- 日期：2026-08-27
- 基线：审计报告 `docs/xui-interaction-audit-windows-baseline.md`（基线提交 `a25aabc`）
- 验收范围：`a25aabc..2de4c96`（6 个修复提交，51 个源文件、约 +2530/-142 行），逐项对照审计清单读代码验证，并运行测试套件
- 结论先行：**验收通过**。审计 10 大高严重度问题中 8 项已修复且质量良好；中严重度约修复一半；测试 104 项在已提交状态全部通过。剩余未修项已在文末整理为 backlog。

## 一、测试结果

- 定向测试：input / menu / tabs / tree_view / list_view / radio / code_command / code_editing / code_selection / terminal / terminal_parser / text_edit / window / split_layout / scrollbar / combobox / date_picker / numeric_input / widget / popup 全部通过。
- 全量套件（104 项）在**当前工作区**（含未提交 WIP）为 102 通过 / 2 失败；失败的两项（`xui_code_editing_test` "delete complete Unicode identifier backward"、`xui_code_edit_test` "shaped hit-test trailing cluster boundary"）经 stash 对照验证**在已提交状态均通过**——失败源于工作区里正在进行的新一轮 Unicode 词边界/命中测试改动（`src/xui_unicode.c`、`src/xui_text_edit.c` 等 20 个未提交文件），不属于本次修复的回归。
- 注：中途出现过一次 `code_command_test` 失败，为陈旧增量构建产物，干净重编译后稳定通过。

## 二、已修复并验证（高严重度 8/10）

| # | 审计项 | 修复情况（验证证据） |
|---|--------|----------------------|
| S-01 | 鼠标光标形状 API | ✅ `XGE_CURSOR_*` + `xgeSetCursor/GetCursor`（`xge.h:384-393`、`src/xge_impl.c` 经 `sapp_set_mouse_cursor`）；XUI 侧 `XUI_CURSOR_*`、widget `onQueryCursor`、`xuiQueryCursor`，代理在每个指针事件后驱动，leave/blur 复位箭头。已注册光标的控件：input（装饰区箭头/文本 I 形）、text_edit、code_edit、rich_edit、terminal（I 形）、split_layout、window、table_view（8 向 resize 箭头）、hyperlink（手型）、dock_panel |
| S-02 | MOUSE_LEAVE / 焦点事件丢弃 | ✅ `XGE_EVENT_MOUSE_LEAVE/WINDOW_FOCUS/WINDOW_BLUR` 全链路转发；blur 时 XGE 重置全部按键/鼠标状态，代理调 `xuiInputCancelAllPointers` + `xuiInputPointerLeave`；focus 清修饰键。悬停卡死、tooltip 滞留、失焦卡按压三缺陷一并消除 |
| S-03 | 交互参数硬编码 | ✅ 新增 `xui_interaction_policy_t`，Win32 代理读取 `GetDoubleClickTime/SM_CXDOUBLECLK/SM_CXDRAG/SPI_GETMOUSEHOVERTIME/GetCaretBlinkTime/SPI_GETWHEELSCROLLLINES`；双击/拖拽判定改为矩形阈值（与 Windows 同构） |
| S-04 | 双击时序/无三击 | ✅ `DOUBLE_CLICK` 改为第二次**按下**时派发（对齐 Windows）；事件携带 `iClickCount`（1/2/3 循环）；拖动/移动后重置点击计数（拖动不再误触发 CLICK，优于审计基线）。终端三击选行改用 `iClickCount==3`，删除了自造计时器 |
| P-01 | 菜单子菜单无延迟 | ✅（部分）新增 0.20s hover 展开延迟（`XUI_MENU_SUBMENU_HOVER_DELAY`，经 onUpdate 计时）。遗留：悬停普通项仍**立即**关闭已开子菜单，无 Windows 的三角轨迹保护；0.2s 也快于 Windows MenuShowDelay 默认 ~0.4s。对角线进入子菜单的体验已明显改善但未完全对齐 |
| D-01/02 | TreeView 双击/右键 | ✅ 双击=选中+展开/折叠（仅限有子节点）；CONTEXT_MENU 右键先选中再回调，Menu 键以焦点行为锚点 |
| D-03/04 | ListView 双击/右键 | ✅ 同上模式，`xuiListViewSetContextMenu` |
| C-01 | Radio 方向键组内导航 | ✅ 上下左右在组内循环移动并选中，跳过禁用/隐藏项，同步聚焦 |
| T-01 | CodeEdit 词级键位 | ✅ Ctrl+Left/Right 词移、Ctrl+Backspace/Delete 删词、Shift+Home/End、Ctrl+Shift+Home/End、Ctrl+Shift+Z 重做全部进默认键表（新增 4 个 SELECT_*_DOCUMENT/LINE 命令），键表单测通过 |

## 三、已修复并验证（中严重度精选）

| 审计项 | 修复情况 |
|--------|----------|
| P-02 菜单项 down 即提交 | ✅ 改为 press→capture→release 同项才提交，可拖出取消 |
| P-07 ComboBox F4/Alt+Up/Down | ✅ 三键齐全，开合切换 |
| P-10 DatePicker 键盘导航 | ✅ 方向键逐日/周跳、PgUp/PgDn 翻月、Ctrl+PgUp/PgDn 翻年、Home/End 周首尾，含 range 双面板与越界钳制 |
| C-02 Tabs 按下选中 | ✅ DOWN 即切换（关闭按钮仍 up 触发，正确） |
| C-03 Ctrl+Tab | ✅ 输入层放行 Ctrl+Tab（不再被焦点循环吞掉），Tabs 接管 Ctrl+Tab/Ctrl+Shift+Tab |
| C-05 Window 双击标题栏 | ✅ 最大化/还原；resize 边缘 8 向光标（依赖 S-01） |
| C-06 SplitLayout | ✅ 双击分隔条均分、方向键微调（8px / Shift 24px）、resize 光标 |
| S-05 Ctrl+Tab（输入层） | ✅ 见上 |
| S-07 Shift+滚轮→水平滚动 | ✅ 输入层统一交换滚轮轴向，全部控件受益 |
| T-02 光标闪烁 | ✅ context 级闪烁时钟：0.53s 半周期（正确理解 GetCaretBlinkTime 语义）、按键/点击/聚焦重置、按需渲染模式下通过 `requestFrame` 维持闪烁。五个编辑控件全部接入 |
| T-04 text_edit 期望列 | ✅ `fPreferredCaretX`，编辑/横向移动时失效 |
| T-05 text_edit PgUp/PgDn | ✅ 按视口行高计算页行数 |
| T-08 IME 组合期保护（部分） | ✅ input/text_edit 在组合期间忽略 TEXT 事件 |
| T-14 Numeric spinner 连发 | ✅ 0.40s 延迟 / 0.08s 间隔 |
| 右键菜单通道（cd88643） | ✅ 补齐 17+ 控件：tabs、tree、list、table_view/grid、property_grid、chart、flow_graph、dock、window、toolbar、statusbar、breadcrumb、tag_input、file_dialog、workflow、inventory、message_list，均带"右键先选中 + Menu 键锚点"语义 |
| S-09 tooltip 延迟 | ✅ 接入 `SPI_GETMOUSEHOVERTIME`（Win 默认 ≈0.4s） |
| 附带质量项 | ✅ TreeView `SetNodes` 事务化；XGE 按需渲染支持延迟帧请求；光标查询回调/交互策略均有公开 API 与参数校验 |

## 四、未修复项（backlog，按建议优先级）

1. **T-11/T-12 终端 Ctrl+C/Ctrl+V 与菜单标签不符**：右键菜单仍标注 "Ctrl+F"（`src/xui_terminal.c:2771`），但 Ctrl+F 实际落入控制字节分支发送 0x06——"标注与行为不符"仍在，且优先级建议高于纯缺失。
2. **S-07 滚轮步长**：`iWheelScrollLines` 已采集但**无消费者**；独立 ScrollBar 仍一格滚一整页（`src/xui_scrollbar.c:913-931` 未改）、ScrollFrame 仍硬编码 48px。
3. **S-06 小键盘**：KP_0-9/KP_ENTER 仍在代理键映射中被丢弃。
4. **D-05 选中时机不统一**：Tabs 改为按下选中，但 TreeView 单击仍为抬起选中（`__xuiTreeViewPointerUp`），库内仍两套标准。
5. **P-03 菜单 Esc 整链关闭**：`src/xui_menu.c:964` 仍 `xuiMenuClose(root)`，与 LEFT 键逐层关闭不一致。
6. **T-03 Input**：Tab 进入仍不全选、无 Enter 提交语义。
7. **P-05/P-04 MenuBar**：无裸 Alt 菜单模式；菜单打开时 RIGHT 仍被用作切换顶级菜单（`src/xui_menubar.c` 本轮未改）。
8. **T-05 rich_edit**：PgUp/PgDn 仍固定 ±10 行（text_edit 已改，rich 未跟进）。
9. **C-04 Window 拖动**：仍为幻影预览 + 释放落位，非实时。
10. **F2**：`XUI_KEY_F2` 全库仍零引用（表格类编辑进入方式）。
11. 低优先级遗留：ComboBox 自动补全（P-08）、list MULTI 模式 Shift 范围选（D-07）、Ctrl+A/marquee（D-08）、表格 Tab/Enter 单元格导航（D-09/D-11）、Input 失焦选区变灰、Insert 覆盖模式、Progress marquee、FlowGraph 滚轮缩放等。

## 五、验收意见

本轮修复方向准确、实现质量高：平台层（光标/焦点事件/系统参数）一次到位且 API 设计克制（policy 可覆盖、cursor 可继承）；输入层双击/三击重构与 Windows 语义对齐；控件层的右键菜单补齐采用了统一的"先选中+键盘锚点"模式，没有出现各控件各自为政的补丁式修法。唯一建议关注的实现细节是菜单子菜单的**关闭侧**仍为即时（见第四节 P-01 遗留），以及滚轮策略字段"采集了但没人用"的半成品状态。

工作区当前还有约 20 个未提交的源码改动（Unicode 词边界/命中测试方向），其中包含 2 个测试失败，提交前需先解决。
