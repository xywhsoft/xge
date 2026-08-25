# XUI 交互体验审计报告（以 Windows 原生交互为基准）

- 日期：2026-08-26
- 范围：`src/` 下 XUI 全部交互类控件 + XGE 平台输入桥接（sokol → XGE → `xuiProxyXgePumpInput` → XUI）
- 方法：逐文件阅读事件处理分支（KEY_DOWN / POINTER_DOWN / UP / MOVE / WHEEL / CLICK / DOUBLE_CLICK / CONTEXT_MENU / DRAG_*），对照 Win32/WinUI 原生控件惯例逐项比对。所有结论均附代码行号，高严重度发现已二次抽查验证。
- 判定分级：[一致] 行为与 Windows 惯例吻合；[不一致] 行为存在但与 Windows 不同；[缺失] Windows 惯例能力不存在；[体验不佳] 行为可用但手感偏差；[超出基准] 优于 Windows 原生。

## 总体结论

XUI 的**架构层输入契约是健康的**：有序事件队列、焦点→热键→冒泡→内置默认的键盘路由、pointer capture/lost、drag 阈值、touch 长按右键菜单、IME 候选窗跟随，这些地基与 Windows 语义基本对齐，且有多份设计文档约束。弹层基础设施（popup 关闭/焦点策略、msgbox 模态）、DockPanel、code_edit 的撤销分组与补全导航甚至超出原生基准。

主要问题集中在三类：

1. **平台桥接缺口**：sokol 的 `MOUSE_LEAVE` / `FOCUSED` / `UNFOCUSED` 事件被丢弃，全链路没有鼠标光标形状 API——这两项在 Windows 上产生肉眼可见的缺陷（悬停卡死、光标永远是箭头）。
2. **事件通道覆盖不全**：`DOUBLE_CLICK`、`CONTEXT_MENU`、`F2` 等事件在 tree_view / list_view / table 系控件中大面积未注册，应用层即使想实现 Explorer 式交互也无从下手。
3. **行为标准不统一**：同类操作在不同控件里时机相反（按下选中 vs 抬起选中；down 提交 vs up 提交）、滚轮步长一套控件一个标准、PgUp/PgDn 三个编辑器三套值。

统计：高严重度 9 项，中严重度约 40 项，低严重度约 60 项，另有约 20 项超出基准的加分项。

---

## 一、系统层（输入管道与平台桥接）

### S-01 无鼠标光标形状 API —— 高 [缺失]

- 全链路（`xui.h`、`xge.h`、`src/xui_proxy_xge.c`、`lib/sokol/sokol_app.h` 的封装层）不存在设置光标形状的调用；sokol 自带的 `sapp_set_mouse_cursor` 未被使用。
- 后果：文本编辑框上不显示 I 形光标；splitter / 窗口边缘 / 表格列边界拖动时无双向箭头；hyperlink 无手型。用户完全失去"此处可交互"的第一反馈通道。各控件审计中所有"光标缺失"项均源于此（`xui_window.c` resize、`xui_split_layout.c` 分隔条、`xui_table_view.c` 列宽边界、`xui_hyperlink.c` 手型）。
- 建议：XGE 增加 `xgeSetCursor(XGE_CURSOR_*)` 转发 `sapp_set_mouse_cursor`，XUI 在 hit-test/hover 变化时按控件声明（widget 属性或 `onQueryCursor` 回调）驱动。

### S-02 MOUSE_LEAVE / FOCUSED / UNFOCUSED 事件被整链丢弃 —— 高 [缺失]

- sokol 已产生这些事件（`sokol_app.h:9750` WM_MOUSELEAVE、`:9645` WM_KILLFOCUS），但 `__xgeSokolEvent`（`src/xge_impl.c:1893-2046`）没有对应 case，落入 `default: break`；`__xgeSokolDispatchSceneEvent` 同样丢弃。`XGE_EVENT_*` 枚举中根本没有 leave/focus 事件，`xuiInputPointerLeave()`（`src/xui_input.c:1442`）在生产路径上**零调用者**。
- 后果一：鼠标移出窗口后 hover 态永远留在最后命中的控件上（按钮保持高亮），tooltip 悬停条件不被打破、持续显示（`xuiInternalTooltipUpdate` 只检查 owner 可见/可用，`src/xui_widget.c:5217-5259`）。
- 后果二：按住按钮时 Alt+Tab / 窗口失焦，XUI 收不到任何"取消"信号：`pActiveWidget` 卡在按压态、拖拽不终止、`g_xge.iMouseButtons` 与 `arrKeyDown[]` 残留脏状态（`src/xge_impl.c` 无任何重置路径）。Windows 对应机制是 WM_CANCELMODE + 捕获丢失。
- 建议：XGE 补 `XGE_EVENT_MOUSE_LEAVE / WINDOW_FOCUS / WINDOW_BLUR`；代理泵接到 `xuiInputPointerLeave` + 合成 pointer cancel + 清空按键状态。

### S-03 双击/拖拽阈值硬编码，不读系统设置 —— 低 [不一致]

- `src/xui_input.c:5-9`：双击 0.45s/4px、拖拽 4px、长按 0.55s/6px 全部硬编码。Windows 基准：`GetDoubleClickTime()`（默认 500ms）、`SM_CXDRAG/SM_CXDOUBLECLK`，且用户可在控制面板调整。0.45s 略快于默认值，对慢速双击用户不友好。
- 建议：平台代理读取一次系统参数注入 XUI context；移动端后端保持现值。

### S-04 双击事件在 mouse-up 之后派发、无三击 —— 中 [不一致]

- `__xuiInputHandleClickEvents`（`src/xui_input.c:1071-1104`）顺序为 UP → CLICK → DOUBLE_CLICK；Windows（CS_DBLCLKS）是第二次 **按下** 时替换为 DBLCLK。
- 影响：
  - 双击激活类操作比 Windows 晚一个 mouse-up 周期；
  - 无法实现 Windows 的"双击后按住拖拽按词扩展选区"（编辑器只见到 UP 后的 DOUBLE_CLICK，拖拽早已开始）；
  - 无三击事件（全库无 TRIPLE 概念），三击选段/选行缺失，`xui_terminal.c:2641-2656` 只能靠自行计时模拟三击——这是对缺陷的正确补偿，但代价是每个控件都要重造一遍；
  - 双击前必先派发一次 CLICK：tree/tabs 等若在 CLICK 里做选中，双击语义叠加需自行防抖。
- 建议：输入层增加 clickCount（1/2/3），在第二次 DOWN 时派发 DOUBLE_CLICK（携带 clickCount=2），保留 CLICK-only 语义兼容；控件按 count 做词/段选择。

### S-05 Ctrl+Tab 被焦点循环吞掉 —— 中 [缺失]

- `xuiInputKeyDownEx`（`src/xui_input.c:1651-1663`）对 TAB 一律 `xuiFocusNext`，不区分 Ctrl。Windows 惯例：Ctrl+Tab/Ctrl+Shift+Tab 在 Tabs/属性表内切换页签。焦点控件理论上可以先消费，但 `xui_tabs.c:955-981` 未处理 TAB，结果 Ctrl+Tab 变成了焦点跳转。
- 建议：内置默认路由跳过带 Ctrl 修饰的 TAB，留给 Tabs/DockPanel 控件注册；Tabs 补 Ctrl+Tab 处理。

### S-06 数字小键盘按键全被丢弃 —— 中 [缺失]

- `__xuiProxyXgeMapKey`（`src/xui_proxy_xge.c:2012-2040`）只映射功能键/方向键与 32-126 的主区字符；sokol 的 `SAPP_KEYCODE_KP_0..KP_ENTER`（320-336）返回 0 直接丢弃。
- 后果：数字小键盘 Enter 不触发默认按钮/提交；NumLock 关闭时小键盘方向键/Home/End 全部无效；依赖 KEY_DOWN 的快捷键在小键盘上失灵（文本输入靠 CHAR 事件侥幸可用）。
- 建议：KP_0-9/KP_ENTER/KP_DECIMAL 等映射到对应 XUI 键。

### S-07 滚轮行为无统一标准，且不读系统配置 —— 中 [不一致]

- 无任何代码读取 `SPI_GETWHEELSCROLLLINES`（Windows 默认 3 行/格），各控件各自为政：
  - 独立 ScrollBar：每格 = LargeStep（**整整一页**）`src/xui_scrollbar.c:913-931`；
  - ScrollFrame：48px/格 `src/xui_scroll_frame.c:606`；
  - text_edit：3 行（恰好等于 Windows 默认）`src/xui_text_edit.c:2654-2656`；
  - code_edit：48px `src/xui_code_edit.c:3620-3623`；terminal：原始增量直传 `src/xui_terminal.c:2876-2881`。
- 全库无 Shift+滚轮→水平滚动的统一处理（`xui_scrollbar.c:919` 只取主轴；`MOD_SHIFT` 在 scroll 家族 0 命中）——Windows 上触控板/无水平轮用户的标准横向滚动方式缺失。
- 建议：context 级统一 wheel 策略：默认 3 行/格（由行高驱动）、Shift 交换轴向、悬停滚动条时仍按行滚动。

### S-08 Enter/Esc 默认动作只沿焦点祖先链查找 —— 中 [缺失]

- `__xuiInputInvokeAction`（`src/xui_input.c:1514-1529`）从焦点控件向上找 `onDefaultAction/onCancelAction`。焦点落在普通控件时，同级注册的"默认按钮"永远不可达——Windows 对话框里 Enter 任何位置都触发默认按钮（`xui_button.c:1140` 注册了默认动作但常常够不到）。MsgBox 依赖窗口自身持有焦点才工作正常。
- 建议：focus scope 级维护 defaultButton/cancelButton 注册表，焦点链查不到时回退到 scope 默认。

### S-09 tooltip 时间参数 —— 低 [不一致]

- 初始延迟 0.35s（`src/xui_widget.c:275`，Windows ≈0.5s），无 5s 自动隐藏、无 0.1s 快速重显；悬停期间无限期显示（无 auto-pop）。msgtip 默认 1.8s、toast 默认 3.0s（Windows 通知 5s）。
- 建议：默认延迟提到 0.5s，加 auto-pop 与重显窗口；toast 允许 `fDuration<=0` 常驻（当前被强制回 3.0s，`src/xui_toast.c:1157` 同签名 if 缺陷）。

### S-10 与 Windows 一致/超出基准的地基能力（正面）

- 点击 = press+release 同控件才触发（`src/xui_input.c:1248-1271`）；按下未拖出语义正确。
- 右键上下文菜单在 **up** 时派发（`:1100-1102`），与 WM_CONTEXTMENU 一致；另有 touch 长按 0.55s 触发（`:906-978`）。
- 滚轮发给悬停命中控件（`:1275-1299`）＝ Win10 1809+ 默认"悬停即滚"。
- 拖拽 4px 阈值 = SM_CXDRAG 默认值；drag begin/move/end/cancel 事件齐全，Esc 取消拖拽在 scrollbar/window/dock/split/joystick 普遍支持（超出 Windows 原生）。
- 禁用控件命中穿透（`XUI_WIDGET_HIT_DEFAULT` 含 ENABLED，`xui.h:398`）≈ WindowFromPoint 跳过 disabled 的语义。
- 键盘路由"焦点控件→全局热键→祖先冒泡→内置默认"有明确契约文档（`docs/xui-input-routing-contract-merge.md`），热键可与编辑器共存。
- IME：TSF 契约清晰，组合串/候选窗跟随光标四个编辑器全部实现，text_edit 甚至构造含组合串的虚拟文档计算候选矩形（`src/xui_text_edit.c:2767-2848`）。
- `XUI_KEY_CONTEXT_MENU`（Menu 键）映射完整（`src/xui_proxy_xge.c:2030`），timeline_view/input_widget 均响应（超出多数 UI 库）。

---

## 二、弹层与浮层控件

### 高严重度

| # | 控件 | 发现 | 位置 | Windows 基准 |
|---|------|------|------|--------------|
| P-01 | Menu | 子菜单 hover **立即**展开/收起，无 400ms 延迟、无对角线三角保护。鼠标斜向移动进入子菜单时，轨迹掠过相邻普通项就会立即收起刚展开的子菜单，实践中极难进入对角子菜单 | `src/xui_menu.c:841-853` | MenuShowDelay ≈400ms + 三角轨迹保护 |

### 中严重度

| # | 控件 | 发现 | 位置 | Windows 基准 |
|---|------|------|------|--------------|
| P-02 | Menu | 菜单项在 mouse-**down** 即提交，按下后无法拖出取消 | `src/xui_menu.c:854-864` | up 激活，可拖出取消 |
| P-03 | Menu | Esc 一次关闭整条菜单链（菜单 handler 抢先于 popup 的 ESCAPE_CLOSE），与 LEFT 键的逐层关闭不一致 | `src/xui_menu.c:899-918` | Esc 每次关一层 |
| P-04 | MenuBar | 菜单打开时焦点被设回 menubar，RIGHT 键变为"切换顶级菜单"，无法用 RIGHT 展开当前项的子菜单 | `src/xui_menubar.c:452,716-745` | RIGHT 展开子菜单 |
| P-05 | MenuBar | 无"裸 Alt 进入/退出菜单模式"；F10 只进不出 | `src/xui_menubar.c:701-714` | Alt/F10 切换 |
| P-06 | Menu | 快捷串只在菜单打开且持有焦点时匹配，未注册全局热键，"显示 Ctrl+S"不代表 Ctrl+S 可用 | `src/xui_menu.c:804-818` | 加速键全局生效 |
| P-07 | ComboBox | 无 F4；关闭态 Down 直接展开下拉且不落选（Windows 是直接改选不展开）；Alt+Up 不收起 | `src/xui_combobox.c:812-848` | F4/Alt+Down/Up 标准三键 |
| P-08 | ComboBox | 无输入自动补全/过滤 | `src/xui_combobox.c:587-608` | 类型选择补全 |
| P-09 | ComboBox | EDIT 模式 Esc 后焦点不回编辑框：恢复目标不可聚焦时 popup 静默放弃恢复 | `src/xui_combobox.c:582` + `src/xui_popup.c:614-620` | 焦点回编辑框 |
| P-10 | DatePicker | 日历网格**完全无键盘导航**：无方向键移日、无 PgUp/PgDn 翻月、无 Ctrl+Home 回今天 | `src/xui_date_picker.c:2166-2172` | 方向键逐日+跨月翻页 |
| P-11 | DatePicker | 点选日期只更新草稿不提交不关闭，必须点 OK（Web 风格取舍，但与 Win32 月历不同） | `src/xui_date_picker.c:1686-1715` | 点选即提交关闭 |
| P-12 | Popup | 通用嵌套弹出：owner 失效只在**下一个事件到达时**惰性关闭，无事件时子层可能残留 | `src/xui_popup.c:505-507,709-711` | 父关子立即销毁 |

### 低严重度（摘要）

Menu 打开无选中记忆（`:1436,1465`）、点击已展开父项不收起（`:616-618`）；MenuBar 助记符无下划线渲染（`:594-602`）；ComboBox 关闭态无滚轮改选（`:1134-1151`）；DatePicker 滚轮不翻月（`:2076-2102`）；IconPicker 点击 down 即提交（`src/xui_icon_picker.c:848-856`）；msgtip 点击关闭且消费点击（`src/xui_msgtip.c:610-613`）；MsgBox Enter 恒返回 button[0]、焦点落在窗口而非默认按钮（`src/xui_msgbox.c:884-894,1345`）；Cascader hover 展开模式无延迟（`src/xui_cascader.c:979-982`）。

### 一致/超出基准（正面）

Popup 外部任意键关闭+消费、modal shield、四向翻转；IconPicker 网格键盘导航完整（含 Home/End/PgUp/PgDn）；DatePicker Esc 逐级撤销；MsgBox 模态屏蔽完整；Cascader 键盘/滚轮完整；MenuBar 划过顶级项立即切换（事件+每帧轮询双保险）。

---

## 三、文本输入与编辑控件

### 高严重度

| # | 控件 | 发现 | 位置 | Windows/VS Code 基准 |
|---|------|------|------|----------------------|
| T-01 | CodeEdit | Ctrl+Left/Right 词移、Ctrl+Backspace/Delete 删词、Shift+Home/End、Ctrl+Shift+Home/End、Ctrl+Shift+Z **均未绑定默认键**（命令已实现，只是没进默认键表） | `src/xui_code_command.c:124-157` | 词移/删词是高频编辑键 |

### 中严重度

| # | 控件 | 发现 | 位置 | 基准 |
|---|------|------|------|------|
| T-02 | 全部编辑器 | 光标**常显不闪烁** | `src/xui_input_widget.c:1996-2004`、`src/xui_text_edit.c:2233-2241`、`src/xui_rich_edit.c:1864-1866` | GetCaretBlinkTime ≈530ms |
| T-03 | Input | Tab 进入焦点不全选；无 Enter 提交语义（仅 onChange，无法区分"完成"与"编辑中"）；无 Esc 恢复 | `src/xui_input_widget.c:2234-2242,2106-2195` | Tab 进入全选、有 commit/cancel |
| T-04 | TextEdit | 上下移动无期望列（goal column）记忆，穿过短行后列位置丢失 | `src/xui_text_edit.c:1664-1665` | 保留首选列（code/rich 均已实现） |
| T-05 | TextEdit/RichEdit | PgUp/PgDn 固定 5/10 行，与视口无关 | `src/xui_text_edit.c:2495-2500`、`src/xui_rich_edit.c:2358-2359` | 按视口页高（code_edit 已正确） |
| T-06 | TextEdit/RichEdit | Tab 不插入制表符（rich 仅列表升降级），焦点被切走 | `src/xui_rich_edit.c:2369-2372` | 多行编辑器插入 Tab |
| T-07 | 全部编辑器 | 无三击选段（源于 S-04；terminal 已自行模拟三击选行） | — | 三击选段 |
| T-08 | 全部编辑器 | IME 组合期间 KEY_DOWN 无保护：组合中方向键/删除键直接改正文 | 各编辑器 KEY_DOWN 分支 | 组合期锁定编辑 |
| T-09 | CodeEdit | 无列选择（全文件无 XUI_MOD_ALT，单选区模型） | `src/xui_code_selection.c:7,135-140` | Alt+拖拽列选 |
| T-10 | CodeEdit | 行号边距点击默认无动作（需应用层自行注册） | `src/xui_code_edit.c:3479-3488` | 点击行号选行 |
| T-11 | Terminal | Ctrl+V 发送 0x16、Ctrl+C 恒为中断（仅 Ctrl+Shift+C/V 可复制粘贴） | `src/xui_terminal.c:2468-2471` | WT 默认 Ctrl+V 粘贴、有选区时 Ctrl+C 复制 |
| T-12 | Terminal | 右键菜单标注 "Ctrl+F" 查找，但实际按键落入控制字节分支发送 0x06——**菜单快捷键标签与实际行为不符** | `src/xui_terminal.c:2748` vs `:2468` | 菜单快捷键可用 |
| T-13 | Terminal | 无右键直接粘贴/无 copy-on-select；Home/End 永发应用序列，无本地 Ctrl+Home/End 滚动；拖选到边缘不自动滚 | `src/xui_terminal.c:2431-2442,2516-2540` | WT 惯例 |
| T-14 | NumericInput | Spinner 按住无连发/加速 | `src/xui_numeric_input.c:581-605` | Windows updown 按住重复 |

### 低严重度（摘要）

无 Insert 覆盖模式；重做缺 Ctrl+Shift+Z（input/text）；失焦选区不变灰（text）；单行框拖到边缘不自动水平滚；rich/code 无拖放移动选中文本；NumericInput 非法输入延迟到提交才报（无键入过滤）、Esc 不恢复原值。

### 一致/超出基准（正面）

Input 的"按在选区内保留选区、up 才收起 + 拖动即移动文本"完整还原 Windows EDIT；四个编辑器右键菜单完整（含动态启停 undo/复制状态）；text_edit 拖选边缘自动滚；code_edit：分组撤销+连续输入合并、PgUp/PgDn 按视口、首选列记忆、Ctrl+双击选行、补全弹窗全键盘导航、minimap 拖拽；rich_edit：Ctrl+滚轮缩放、表格单元格双击编辑、期望列记忆；terminal：三击选行自主模拟、用户回滚时暂停自动跟随、Alt+方向键 xterm 序列完整；tag_input：Enter/逗号确认、空输入退格删标签。

---

## 四、数据选择类控件

### 高严重度

| # | 控件 | 发现 | 位置 | Windows 基准 |
|---|------|------|------|--------------|
| D-01 | TreeView | 未注册 `XUI_EVENT_POINTER_DOUBLE_CLICK`：双击节点不展开/折叠（只产生两次选中） | `src/xui_tree_view.c:1517-1534` | Explorer 双击展开 |
| D-02 | TreeView | 无 CONTEXT_MENU 处理且右键 down 只认左键：右键既不选中也不通知——应用层无法实现"右键先选中再弹菜单" | `src/xui_tree_view.c:894,1517-1534` | 右键先选中 |
| D-03 | ListView | 同上：无双击事件（双击只触发两次 select） | `src/xui_list_view.c:1132-1148` | NM_DBLCLK 双击激活 |
| D-04 | ListView | 同上：右键不选中、无 CONTEXT_MENU | `src/xui_list_view.c:731` | 右键先选中 |

### 中严重度

| # | 控件 | 发现 | 位置 | 基准 |
|---|------|------|------|------|
| D-05 | TreeView | 选中发生在 mouse-**up**（库内 list/table/inventory/message 均为按下选中，自相矛盾） | `src/xui_tree_view.c:951` | 按下选中 |
| D-06 | TreeView | checkbox 节点空格不切换勾选（Space/Enter 只重发 select 通知） | `src/xui_tree_view.c:1064-1070` | 空格切换 |
| D-07 | ListView | Shift 范围选仅在 RANGE 模式生效，MULTI 模式下 Shift+点击退化为普通单击（inventory 的 MULTI 却同时支持 Ctrl+Shift——同库两套标准） | `src/xui_list_view.c:630-638` vs `src/xui_inventory_grid.c:1026-1040` | 多选模式 Ctrl/Shift 均有效 |
| D-08 | ListView | 无 Ctrl+A 全选；无空白处 marquee 框选 | `src/xui_list_view.c:716-834` | Ctrl+A / 橡皮筋 |
| D-09 | TableView | 双击列边界不自适应列宽；无 F2 编辑；Tab/Enter 不在单元格间移动 | `src/xui_table_view.c:2036-2047,1491-1572` | 双击自适应 / F2 / Tab 移格 |
| D-10 | TableView | BOOL 单元格绘制成复选框但点击只选中不切换 | `src/xui_table_view.c:1059-1065,1444-1450` | 复选框列单击切换 |
| D-11 | TableGrid | 编辑中 Tab 移格缺失（焦点被移出控件）；无 F2 | `src/xui_table_grid.c:1007-1035` | Tab 提交并右移 |
| D-12 | InventoryGrid | 单击 pointer-up 也触发 Activate，与双击 Activate 不可区分 | `src/xui_inventory_grid.c:1841-1843,2184-2193` | 单击选中、双击激活 |

### 低严重度（摘要）

`XUI_KEY_F2` 在全部 src/*.c 中零引用；TableView 列头排序在 down 触发（Windows 抬起）、无固定列、无多选拖选；PropertyGrid 分类行 down 即折叠、无列宽拖动、自身无键盘；TreeView 无展开动画、无多选、`SetNodes` 后选中丢失；Chart 无右键/双击复位、两轴同缩放；Timeline 右键不先选中命中帧、拖刷阈值 3px 与平台 4px 不一致；MessageList 无方向键导航；Carousel 悬停不暂停自动播放、无滚轮/滑动手势；Breadcrumb 无键盘。

### 一致/超出基准（正面）

TreeView 左右键语义完全正确（折叠时焦点留本项、再按才到父项）；ListView/TableView/Inventory 方向键+PgUp/PgDn+Home/End+EnsureVisible+焦点框容器门控全部正确；InventoryGrid 右键先选中+拖放（MOVE/STACK/SWAP 建议+预览）+gamepad 支持超出基准；Accordion 键盘全套；Chart 光标锚点滚轮缩放+拖拽平移+Shift 框选；MessageList 跨节点文本拖选+边缘自动滚+右键全选复制。

---

## 五、容器与导航类控件

### 高严重度

| # | 控件 | 发现 | 位置 | Windows 基准 |
|---|------|------|------|--------------|
| C-01 | Radio | KEY_DOWN 仅处理 SPACE：**无方向键组内导航**（radiogroup 类型存在但无键盘语义）——键盘用户无法在选项间移动 | `src/xui_radio.c:659-672` | 上下/左右在组内移动并选中 |

### 中严重度

| # | 控件 | 发现 | 位置 | 基准 |
|---|------|------|------|------|
| C-02 | Tabs | 选中发生在 mouse-**up**（Windows TabCtrl 按下即切页；库内 DockPanel 标签就是按下选中） | `src/xui_tabs.c:710-741` | 按下切换 |
| C-03 | Tabs | 无 Ctrl+Tab（见 S-05）；滚轮滚动标签条而非切换标签；无拖动重排 | `src/xui_tabs.c:886-899,955-981` | Ctrl+Tab 切页、滚轮切标签 |
| C-04 | Window | 标题栏拖动/边缘 resize 为"幻影预览+释放落位"，非实时移动 | `src/xui_window.c:1180-1216` | 实时跟随 |
| C-05 | Window | 无双击标题栏最大化/还原（全文件无 DOUBLE_CLICK）；Esc 不关闭窗口（未接 onCancelAction） | `src/xui_window.c` | 双击最大化、Esc=取消 |
| C-06 | SplitLayout | 分隔条拖动为阴影预览、释放才提交；无双击均分重置；divider 可聚焦却无方向键微调、无 F6 | `src/xui_split_layout.c:928-960,1040-1045` | 实时拖动、双击重置、键盘可调 |
| C-07 | Button | 无对话框级默认按钮（见 S-08） | `src/xui_button.c:1140` | Enter 全局触发默认按钮 |
| C-08 | Hyperlink | 手型光标缺失（源于 S-01）；无 Ctrl+点击区分 | `src/xui_hyperlink.c` | 手型 |
| C-09 | FlowGraph | 无滚轮缩放（viewport 的 Zoom/Pan 仅 API 字段）、无画布平移手势、无右键菜单、无方向键微移节点 | `src/xui_flow_graph_widget.c:1444-1448,2215-2225` | 滚轮缩放/空格平移为图编辑器标配 |
| C-10 | Progress | 无 marquee 不定态模式 | `src/xui_progress.c`（grep 无 marquee） | PBS_MARQUEE |

### 低严重度（摘要）

Window 移动被完全钳制在父区域内不能拖出（`:481-517`）；Tabs 无中键关闭/双击；Radio 选中在释放（Windows 按下）；Statusbar 无 size grip；Joystick 无键盘操控；FlowGraph 节点拖动无阈值（1px 即开始挪动）。

### 一致/超出基准（正面）

Button 空格"按下态+释放触发"、移出释放不触发完全正确；Checkbox 空格 key-up 触发；Slider 全套（轨道点击跳位、实时拖动、方向键/PgUp/Home/End/Esc、滚轮）超出 Win32 trackbar；RangeSlider 双 thumb+键盘联动；ScrollBar thumb 实时比例跟踪、轨道翻页+按住连滚（0.35s/0.06s）、箭头按住连滚、Esc 取消拖动全部正确；DockPanel（按下选标签、拖出浮动、半透明 dock 预览、中心合并、中键关闭、Ctrl 强制浮动、Esc 取消）是全库交互质量最高的复合控件；Toolbar 键盘+溢出菜单；Joystick 多指捕获+释放回中。

---

## 六、横向一致性问题（同库不同标准）

1. **选中/提交时机三套标准并存**：
   - up 触发：tabs、tree_view、accordion、carousel、breadcrumb（click 语义派）
   - down 触发：list_view、table_view、inventory、message_list 的选中，以及 menu/icon_picker/toast/property_grid 的提交
   - Windows 惯例：列表类**按下**选中；菜单**抬起**激活。当前 menu 与 iconpicker 的 down 提交、tree 的 up 选中都各有与基准相反者。
2. **滚轮步长五个值**：整页（scrollbar）/48px（scroll_frame、code_edit）/3 行（text_edit）/原始增量（terminal）/单步（slider、numeric）。
3. **PgUp/PgDn 三个标准**：text_edit 固定 5 行、rich_edit 固定 10 行、code_edit/tree/terminal 按视口。
4. **拖动阈值不统一**：平台 4px、timeline 刷选 3px（`src/xui_timeline_view.c:1137`）、input 选区判定 ~6px、flowgraph 节点无阈值。
5. **多选语义两套**：list_view MULTI 模式 Shift 失效 vs inventory MULTI 模式 Ctrl/Shift 全支持。
6. **双击/右键通道覆盖不均**：table_grid/timeline/inventory/message_list 已用 DOUBLE_CLICK/CONTEXT_MENU，tree_view/list_view/table_view/chart 完全未接。

---

## 七、修复优先级建议

按"影响面 × 用户感知频率"排序：

1. **S-01 光标形状 API**：一次投入，全部控件受益，是 Windows 桌面观感差距最大的一项。
2. **S-02 leave/focus 事件接线**：消除悬停卡死、tooltip 滞留、失焦卡按压三个可见缺陷。
3. **P-01 菜单子菜单延迟+三角保护**：菜单是高频入口，当前几乎无法用鼠标进入对角子菜单。
4. **D-01~D-04 tree/list 的 DOUBLE_CLICK + CONTEXT_MENU 通道**：补两个事件注册即可让应用层实现 Explorer 式交互。
5. **C-01 Radio 方向键组内导航**：键盘可访问性硬缺口，实现成本低。
6. **T-01 code_edit 默认键表补词移/删词**：命令已存在，纯键表补全。
7. **T-11/T-12 终端 Ctrl+C/V 与菜单标签对齐**：标注与行为不符属于"欺骗用户"，优先级高于纯缺失。
8. **S-07 滚轮标准统一**（含 Shift+滚轮水平滚动）。
9. **统一选中时机为"按下选中"**（D-05、C-02），menu/iconpicker 改回 up 提交（P-02）。
10. **T-02 光标闪烁**（四编辑器统一，读 GetCaretBlinkTime）。

中期项：S-04 三击与 clickCount、S-05 Ctrl+Tab、S-06 小键盘、P-10 DatePicker 键盘、C-04/C-06 实时拖动、D-09/D-11 表格 F2/Tab 导航、S-08 默认按钮注册表。

---

## 附：审计覆盖文件清单

- 平台/核心：`src/xge_impl.c`、`src/xge_input.c`、`src/xui_proxy_xge.c`、`src/xui_input.c`、`src/xui_core.c`、`src/xui_widget.c`（tooltip/capture/IME 段）、`lib/sokol/sokol_app.h`（Win32 段比对）
- 弹层：popup、menu、menubar、combobox、cascader、date_picker、color_picker、icon_picker、msgtip、toast、msgbox
- 文本：input_widget、text_edit、code_edit（含 code_command/code_editing/code_selection/code_document）、rich_edit、tag_input、numeric_input、terminal
- 数据：tree_view、list_view、table_view、table_grid、property_grid、inventory_grid、accordion、chart、timeline_view、message_list、carousel、breadcrumb
- 容器：tabs、window、dock_panel、split_layout、scrollbar、scroll_frame、scroll_view、button、checkbox、radio、slider、range_slider、toggle、progress、hyperlink、toolbar、statusbar、flow_graph(_widget)、workflow、virtual_joystick
