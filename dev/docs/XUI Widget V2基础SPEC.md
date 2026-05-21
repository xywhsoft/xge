# XUI Widget V2 基础 SPEC

本文档跟踪 2026-05-07 起 XGE 内置 XUI Widget V2 基础层重构进度。旧 SPEC 保留历史记录；涉及 clip、Z、事件、焦点、滚动、IME、box model 和控件成熟度的验收，以本文为准。

> 2026-05-09 口径更新：阶段 E 已完成的是原始事件的 tunnel/target/bubble 路由。MouseEnter、MouseLeave、MouseMove、MouseDown、MouseUp、Click、DoubleClick、Key、HotKey、Command 等基础语义事件，以及 eventMask/subtreeEventMask 性能机制，纳入阶段 E2 跟踪。本轮只升级 Widget 和基础设施层，具体控件逐个重构接入。

> 2026-05-09 口径更新：当前已有 `paintBefore` / `procPaint` 低层回调，但还不是正式 OwnerDraw。自定义重绘纳入阶段 H2 跟踪，默认口径是替换控件内容绘制，同时保留 Widget 基础背景、clip、children、border、focus ring、disabled overlay 和 debug outline。

关联设计文档：

- `dev/docs/XUI Widget V2基础设计.md`
- `dev/docs/XUI轻量控件与布局SPEC.md`
- `dev/docs/XUI控件标准与扩展设计.md`
- `dev/docs/XUI控件标准与扩展SPEC.md`
- `dev/docs/XUI声明式界面与样式设计.md`
- `dev/docs/XUI声明式界面与样式SPEC.md`
- `dev/docs/XGE_XUI分离路线.md`

## 进度维护规则

每次开始相关开发前，必须先更新本文档进度：

1. 将即将开发的任务从 `[ ]` 改为 `[~]`。
2. 如果任务范围变化，先调整任务描述或新增子任务。
3. 如果发现阻塞，追加到“阻塞与决策记录”。
4. 开发完成、构建通过、示例或测试通过后，才能改为 `[x]`。
5. 如果任务被放弃、替代或暂缓，标记为 `[!]` 并写明原因。

状态含义：

- `[ ]` 未开始。
- `[~]` 进行中。
- `[x]` 已完成，并通过必要验证。
- `[!]` 放弃、替代或暂缓，必须说明原因。

## 总体验收

- [x] Widget 基类统一 layout、box model、clip、Z、event route、focus、tab、capture、IME、dirty 和基础绘制。说明：基础绘制的 background、border、radius、focus ring、disabled overlay、debug outline 已进入 widget style/paint；旧控件重验已完成；控件初始化中的 role、focus/tab、IME 默认策略已收敛到内部 widget role policy helper，普通控件、文本控件、viewport 和 overlay 不再各自手写同一套基类策略。
- [x] Paint、hit test、事件目标选择使用一致的 `layer > zIndex > treeOrder`。说明：同父级 child paint、hit test 和 point event target 已使用同一排序；事件回调已携带 phase/target 元数据。
- [x] 所有文字绘制默认不会溢出控件本体，除非控件显式允许 overflow visible。说明：Control role 默认同步 `overflow: clip`，host 文本/图片绘制只在 widget clip 有效时强制裁剪；`overflow: visible` 会取消基础层注入的 `XGE_TEXT_CLIP`。
- [x] 控件事件按 tunnel、target、bubble 路由；无人消费时返回 XGE。说明：`procCaptureEvent` 作为 tunnel，target/bubble 走 `procEvent`；`continue`、`handled`、`consumed` 三档结果已落地，无人处理返回 `XGE_XUI_EVENT_CONTINUE`，普通树和 modal overlay 子树均有路由回归。
- [x] Widget 基础语义事件达到常规应用开发可用状态。说明：MouseEnter/Leave/Move/Down/Up/Wheel、Click、DoubleClick、ContextMenu、KeyDown/Up、TextInput、HotKey、Command、Got/LostFocus、CaptureLost/Cancel、DragBegin/Move/End/Cancel 和状态变化通知已进入基础设施；没有事件兴趣时会跳过 MouseMove、hover、tooltip、hotkey 等热路径成本。
- [x] Pointer capture 支持 drag、lost capture 和控件销毁/隐藏/禁用自动释放。说明：pointer 0 兼容 capture、按 pointer id capture 表、move/up/cancel 优先送达、lost/cancel 通知、隐藏/禁用/移除/销毁自动释放已落地；内置拖拽/按压控件已按事件 `iPointerId` 设置、查询和释放 capture。
- [x] FocusManager 统一管理焦点、TAB、overlay focus scope、focus restore 和 IME。说明：单一 focus、`focusable`、`tabStop`、`tabIndex`、TAB 遍历、overlay/modal TAB scope、命名 focus scope、Enter default action、Escape cancel action、`imeMode` 焦点同步、候选框位置缓存和 Popup/Window focus restore 已落地；Input/TextEdit 通过 widget 候选框解析器接入，普通 widget 默认使用 content rect。
- [x] ScrollViewBase / VirtualScrollViewBase 被 ScrollView、ListView、TreeView、TableView、PropertyGrid 等控件复用。说明：ScrollView 已直接复用 ScrollViewBase，virtualized ListView 已直接复用 VirtualScrollViewBase，ListView、TreeView、TableView、PropertyGrid 已内嵌 `tBase` 复用 VirtualScrollViewBase 通用状态。
- [x] XSON 同步 Widget role、children、overflow、z、scroll、IME 字段和错误路径。说明：type -> role、Control children 拒绝、Viewport/virtualized ListView children 规则、overflow/z/layer/focus/IME/ScrollViewBase 字段和错误路径均已进入回归。
- [x] Widget OwnerDraw 基础设施达到可用于常规控件与编辑器/工具界面自绘场景。说明：Widget 级 PaintInfo、OwnerDraw mode、PaintAfter、内容/子树/完整替换模式、dirty 子树清理、公开 guide 和回归测试已完成。
- [ ] 复合控件 item/cell OwnerDraw 逐个接入。说明：List/virtualized ListView/Tree/Table/PropertyGrid 等控件按后续重构逐个补齐，不再作为 Widget 基础设施阻塞项。
- [x] 旧控件全部按 Widget V2 重验，历史 `[x]` 不直接等同成熟。说明：阶段 L 已覆盖当前 `src/xge_xui_*.c` 中的公开控件；`core/layout/host/page/text` 归入基础设施，不按控件成熟度单列。

## 阶段 0：文档口径收敛

- [x] 新增 `XUI Widget V2基础设计.md`。
- [x] 新增 `XUI Widget V2基础SPEC.md`。
- [x] 更新 `XUI轻量控件与布局SPEC.md`，去除与 Widget V2 冲突的暂缓/核心控件旧口径。
- [x] 更新 `XUI控件标准与扩展设计.md`，声明 Widget V2 为控件成熟度前置条件。
- [x] 更新 `XUI控件标准与扩展SPEC.md`，增加 Widget V2 前置阶段。
- [x] 更新 `XUI声明式界面与样式设计.md`，同步 role、children、ScrollViewBase 字段。
- [x] 更新 `XUI声明式界面与样式SPEC.md`，增加 Widget V2 XSON 同步阶段。
- [x] 更新 `XGE_XUI分离路线.md`，把 Widget V2 作为剥离前置条件。
- [x] 更新公开 guide，避免新控件开发继续引用旧基础层口径。

## 阶段 A：Widget 数据结构与 Role

- [x] 增加 Widget role：Control、Container、Viewport、Overlay。
- [x] 明确默认 role 和创建 API。
- [x] 为 Control 默认关闭用户 children 语义。说明：XSON 用户 children 已严格拒绝；公共 `xgeXuiWidgetAdd` 会拒绝 role 为 Control 的 parent，XUI 内部 slot/组合装配改走内部 add 通道。
- [x] 为 Container 开启 children 和 child layout 语义。
- [x] 为 Viewport 开启 content clip、scroll offset 语义。说明：ScrollViewBase 初始化会设置 Viewport role 与 `overflow: scroll`，布局阶段识别 ScrollViewBase/ScrollView 并对子树应用 scroll offset。
- [x] 为 Overlay 接入 overlay root / layer 语义。说明：overlay root、popup、Window、toast 等已标记为 Overlay，统一通过 `layer > zIndex > treeOrder` 排序，并由 OverlayManager 挂载到独立 overlay root。
- [x] 增加 role 违反规则的 debug assert 或 loader 错误。
- [x] 更新所有内置控件初始化默认 role。
- [x] Tooltip 收敛为 widget 内部机制，支持静态描述、NULL 清空、动态 resolver 和 context 内部 overlay popup。
- [x] Toolbar item tooltip 通过 widget 动态 resolver 自动显示，不再需要独立 Tooltip 控件实例。

## 阶段 B：盒模型与布局语义

- [x] 定义 outerRect、borderRect、paddingRect、contentRect。
- [x] 统一 margin、border、padding 对 measure/arrange 的影响。说明：margin 影响父布局占位；border width 与 padding 共同影响 contentRect，并进入 content-size measure。
- [x] 统一 content size 计算。
- [x] 更新 Row、Column、Stack、Grid、Dock 使用 contentRect 布局 children。
- [x] 更新 hit test 默认使用 borderRect。
- [x] 增加 layout snapshot 覆盖四层 rect。
- [x] 增加控件最小尺寸、content 尺寸和 min/max 回归测试。

## 阶段 C：PaintContext 与 Clip

- [x] 增加 PaintContext clip stack。
- [x] Widget paint 统一执行 push clip、draw background/border、control paint、children paint、pop clip。
- [x] 实现 overflow：visible、hidden、clip、scroll。说明：widget/style/XSON 字段、public setter/getter、paint clip、hit test 映射和非法枚举回退已落地；`scroll` 表示显式滚动视图语义，不自动为普通 widget 创建滚动条。
- [x] 普通 Control 默认裁剪文字和内容。说明：Control role 默认启用 widget contentRect clip，文本与图片 host 绘制遵循当前 clip；显式 `overflow: visible` / `clip=false` 会放开这层基础裁剪。
- [x] 所有文本绘制路径接入当前 clip。说明：XUI 内部文本绘制统一走 `__xgeXuiHostDrawTextRect`，基础 host 会在 active Control clip 有效时接入 PaintContext clip stack，并在 overflow visible 时取消强制文本裁剪。
- [x] ScrollViewBase viewport clip 接入 PaintContext。说明：`xge_xui_scroll_view_base_t` 默认使用 widget `overflow: scroll` 接入 contentRect clip。
- [x] ScrollViewBase 滚轮轴向、内容拖拽、滚动条拖拽、nested scroll 策略。说明：ScrollViewBase 已支持 `wheelAxis`、`contentDrag`/`dragMode`、`scrollbarDrag`、`nestedScroll`。
- [x] Overlay layer 绘制不破坏普通树 clip stack。说明：普通树 paint 结束后会恢复外部 clip；overlay root 随后按自身 clip stack 绘制，并在结束后恢复外部 clip。
- [x] 增加文字溢出、子树溢出、嵌套 clip 回归测试。说明：Control 文本/图片默认裁剪、overflow visible 放开、父级 clip 子树溢出、嵌套 clip 和外部 clip 恢复已覆盖。

## 阶段 D：Z 序与 Overlay Layer

- [x] 定义 layer：Normal、Floating、Popup、Modal、Tooltip、DragAdorner、Debug。说明：widget/style/API 常量已落地；OverlayManager 已完成 attach/owner/top/bring-to-front 和 XSON portal 第一轮；debug layer 已覆盖低 z 仍压过 DragAdorner 高 z 的排序验证。
- [x] 定义 `zIndex` 与 `treeOrder`。说明：现有 `xgeXuiWidgetSetZ` 作为 zIndex 入口；widget add 会写入 treeOrder，并已有顺序回归。
- [x] 绘制排序统一使用 `layer > zIndex > treeOrder`。说明：同父级 child paint 已按该规则排序，含 Debug > DragAdorner 的专项回归。
- [x] Hit test 排序统一使用同一规则。说明：同父级 child hit test 已按该规则排序，含 Debug > DragAdorner 的专项回归。
- [x] 事件 target 选择统一使用同一规则。说明：point event target 基于 hit test，已继承同一排序，含 debug layer 鼠标 target 回归。
- [x] OverlayManager 统一 Popup、Menu、ComboBox、Window 和 widget tooltip 浮层。说明：已新增 `xgeXuiOverlayAttach` / `Detach` / `BringToFront` / `GetOwner` / `Top`，统一 overlay root 挂载、owner 元数据、layer 排序和同层 treeOrder 提升；Menu、ComboBox 和 widget tooltip 浮层已走统一挂载，Popup/Window 打开时接入 bring-to-front；XSON `popup`/`Window`/`MsgBox` 声明节点已通过 portal 进入 overlay root。
- [x] 移除控件内部手写 z base / 补丁式遮盖逻辑。说明：Popup/Window/Menu/ComboBox/Tooltip 不再维护独立 z base，默认排序由 `layer > zIndex > treeOrder` 接管；手动同层排序统一使用 `xgeXuiWidgetSetZ`。
- [x] 增加 overlay 叠放、modal、tooltip、debug layer 回归测试。说明：layer 排序、XSON layer/z、Popup/Menu/ComboBox overlay top、Window modal 独占、Tooltip layer、OverlayManager owner 清理、XSON portal 卸载、overlay clip stack 和 debug layer paint/hit/event target 已覆盖。

## 阶段 E：事件路由

- [x] 定义事件 phase：tunnel、target、bubble。
- [x] 定义处理结果：continue、handled、consumed。说明：`continue` 表示无人处理并继续交给 XGE；`handled` 表示已处理但继续当前路由，最终阻止 XGE fallback 和 default/cancel action；`consumed` 表示立即停止当前路由。
- [x] 实现 root-to-target tunnel。
- [x] 实现 target handler。
- [x] 实现 target-to-root bubble。
- [x] 无 Widget 消费时返回 XGE。
- [x] 保留 originalTarget、currentTarget、phase、capture 信息。
- [x] 支持 inputTransparent / hitTestVisible。
- [x] 增加嵌套控件、overlay、disabled/hidden、透明穿透回归测试。说明：嵌套路由、capture、disabled/hidden、inputTransparent/hitTestVisible、modal overlay 子树 tunnel/target/bubble 和 overlay outside 预处理隔离均已覆盖。

## 阶段 E2：Widget 语义事件与热路径优化

- [x] 决策：本阶段只升级 Widget 与基础设施层；具体控件后续逐个重构接入，不在本阶段一次性修改所有控件行为。
- [x] 定义 Widget 基础语义事件枚举与事件结构。说明：已覆盖 MouseEnter、MouseLeave、MouseMove、MouseDown、MouseUp、MouseWheel、Click、DoubleClick、ContextMenu、KeyDown、KeyUp、TextInput、HotKey、Command、GotFocus、LostFocus、CaptureLost、CaptureCancel、DragBegin、DragMove、DragEnd、DragCancel、BoundsChanged、VisibleChanged、EnabledChanged。
- [x] 保留原始 XGE 事件路由作为基础。说明：语义事件携带 originalTarget、currentTarget、phase、pointer id、button、modifier、capture、timestamp 等路由元数据；控件代理转发不得丢失这些信息。
- [x] 增加事件注册 API 与回调存储策略。说明：支持按事件类型注册/清理回调；普通 widget 未注册事件时不增加 dispatch 热路径成本。
- [x] 增加 `eventMask` 与 `subtreeEventMask`。说明：自身 mask 表示本 widget 的事件兴趣；子树 mask 由子节点变更增量传播，用于快速判断某类事件是否需要 hit test、hover 维护或路由。
- [x] 改造 MouseMove 热路径。说明：没有 capture、drag、hover、tooltip、cursor、drop target 或 move interest 时，MouseMove 不做完整 hit test、不创建语义事件、不扫描子树。
- [x] 增加 MouseMove 合并策略。说明：事件队列会合并队尾连续的同类型、同 pointer、同 modifier/button 状态的 `MouseMove` / `TouchMove`；保留最后坐标并累加 delta，不跨越 down/up/key/text 等边界事件，也不合并不同 pointer。
- [x] 改造 hover path。说明：由单一 hover widget 升级为 root-to-target hover path；父容器和子控件均能收到正确 MouseEnter/MouseLeave，overlay/modal 变化、隐藏、禁用、销毁时能补发离开。
- [x] 定义 Click 语义。说明：同一 pointer 在同一 widget 或其子树有效点击区域内 left down/up，且未超过拖拽距离阈值时合成 `XGE_EVENT_XUI_CLICK`；外部释放、移动超阈值、隐藏/禁用/移除/销毁会取消点击状态。
- [x] 定义 DoubleClick 语义。说明：同一 button、同一 target、同一 pointer 在 0.50 秒和 6px 距离阈值内第二次点击成立；第二次点击会先产生 `Click`，再产生 `DoubleClick`，控件层可自行抑制第二次 Click。
- [x] 定义 ContextMenu 语义。说明：右键点击、长按、Menu 键和 Shift+F10 已合成 `XGE_EVENT_XUI_CONTEXT_MENU`；长按与右键复用 context begin/end/cancel 路由，键盘触发以当前 focus target 为目标。
- [x] 定义键盘事件分层。说明：`KeyDown` / `KeyUp` 按物理按键、modifier 路由；`TextInput` 由 `XGE_EVENT_TEXT` 和 IME composition 提交路径承载 codepoint 文本输入；三者分别注册、分别清理，keyboard mask 只用于兴趣聚合，不混用物理按键和文本输入。
- [x] 实现 HotKey 注册表。说明：新增 context 级固定容量注册表和 `xgeXuiHotKeyRegister` / `xgeXuiHotKeyUnregister` / `xgeXuiHotKeyClearWidget`；按 key+modifier 直接匹配，再按 modal/focus scope、visible/enabled 和当前 tree 归属过滤，禁止按键时全树扫描。
- [x] 实现 Command 分发。说明：新增 `xge_xui_command_t`、`xgeXuiCommandDispatch` 和 `xgeXuiHotKeyRegisterCommand`；Command 事件携带数字 id、字符串 name、source widget 和业务数据，HotKey 可直接触发同一 command，Widget 基础层只负责路由，不绑定具体控件业务状态。
- [x] 实现基础拖拽事件。说明：新增 `xgeXuiWidgetSetDragEnabled` / `xgeXuiWidgetIsDragEnabled` 和 DragBegin/Move/End/Cancel 状态机；只在 widget 显式启用或注册 Drag 事件时进入热路径，按下后超过 6px 阈值才开始拖拽，开始后通过 pointer capture 接管后续 move/up/cancel，不默认抢占普通 MouseMove 或左键语义。
- [x] 接入 Tooltip event interest。说明：设置静态 tooltip 或动态 resolver 自动加入 tooltip interest；tooltip 清空或 resolver 设为 NULL 后清除对应 interest，避免无人提示时仍维护 hover 成本。
- [x] 增加状态变化通知。说明：`BoundsChanged`、`VisibleChanged`、`EnabledChanged` 已作为 opt-in widget-local 语义事件接入；同值设置不触发，隐藏/禁用/移除/销毁仍会先清理 capture、focus、hover、tooltip、click/context press 和 drag state 引用；移除/销毁会同步清理 HotKey/Command 注册并刷新父级 subtreeEventMask。
- [x] 增加调试与 trace。说明：`xgedbgXuiEventTrace` 已作为 debug-only API 输出事件命中、capture、eventMask/subtreeEventMask、drag state 和 hotkey/command 匹配结果；release 头文件不声明，`xge.lib` 不导出，运行时不保留额外热路径开销。
- [x] 增加语义事件回归测试。说明：已覆盖嵌套 enter/leave、MouseMove 无兴趣跳过 hit test、有兴趣命中、MouseMove 队列合并、eventMask/subtreeEventMask、keyboard/text split、hotkey 注册/注销/禁用跳过、command 直接派发、hotkey 转 command、drag begin/move/end/cancel、click/double click、context menu、state change、tooltip interest、tooltip resolver、tooltip owner 隐藏/禁用清理，hover/click/context press/drag press 隐藏/禁用清理，以及 remove/free 对 capture、drag state、HotKey/Command 注册表和 subtreeEventMask 的清理。
- [x] 更新基础设施 guide。说明：新增 `docs/guide/xui-widget-events-intro.md`，说明 raw XGE event、Widget 基础语义事件和控件业务事件的边界，覆盖处理结果、热路径 opt-in、Command/HotKey、Drag/Capture、状态清理和 `xgedbgXuiEventTrace`。

## 阶段 F：Focus、TAB 与 IME

- [x] FocusManager 统一 focused widget。说明：context 已维护单一 focus，隐藏/禁用会释放焦点；TAB 和 default/cancel action 已接入 overlay/modal focus scope 与命名 focus scope；IME 开关和候选框位置随焦点统一刷新。
- [x] 支持 focusable、tabStop、tabIndex。
- [x] 支持 focus scope。说明：TAB 已支持可见 modal overlay 限制到最上层 modal 子树；非 modal overlay 在当前焦点位于该 overlay 子树时限制在该 overlay 内；普通 widget 可通过 `xgeXuiWidgetSetFocusScope` 声明命名 focus scope，当前焦点位于该 scope 子树时，TAB 与 default/cancel action 限制在最近可见启用 scope 内。
- [x] 支持 Enter default action。说明：widget 可通过 `xgeXuiWidgetSetDefaultAction` 设置默认动作；键盘事件无人消费时，FocusManager 在当前 focus scope 内按 TAB 顺序触发第一个可见启用 default action。
- [x] 支持 Escape cancel action。说明：widget 可通过 `xgeXuiWidgetSetCancelAction` 设置取消动作；键盘事件无人消费且没有 pointer capture 时，FocusManager 在当前 focus scope 内按 TAB 顺序触发第一个可见启用 cancel action。
- [x] 支持 Popup/Window focus restore。说明：Popup/Window 支持自动捕获当前焦点并在关闭后恢复；自动 restore 在关闭后清空，避免重复打开恢复到过期焦点；显式 `SetFocusRestore` 继续保留，用于 Menu/ComboBox owner 恢复。
- [x] 支持 overlay/modal 限制 TAB 范围。
- [x] 定义 imeMode：disabled、enabled、auto。
- [x] 普通非文本控件默认 disabled IME。
- [x] Input/TextEdit 默认申请 IME。
- [x] Password 默认禁用 IME。
- [x] focus 变化时统一调用 XGE IME 开关和候选框位置更新。说明：焦点变化已统一调用 `xgeImeSetEnabled` 并刷新 context candidate rect；widget 可注册 `xgeXuiWidgetSetImeCandidateRect`，Input/TextEdit 已自动注册光标位置解析器。
- [x] 增加焦点遍历、modal、IME 开关回归测试。说明：TAB 顺序、tabStop 排除、手动 focus、overlay/modal TAB scope、命名 focus scope、default/cancel action、imeMode 焦点同步、candidate rect、自定义解析器、Password/Input/TextEdit 默认策略已覆盖。

## 阶段 G：Pointer Capture

- [x] 支持按 pointer id capture。说明：新增 `xgeXuiSetPointerCapture` / `xgeXuiGetPointerCapture` / `xgeXuiHasCapture` / `xgeXuiWidgetHasCapture`，`xge_event_t.iPointerId` 参与坐标事件 capture 查找，pointer 0 保持 `xgeXuiSetCapture` 兼容口径。
- [x] move/up/cancel 优先送达 capture widget。
- [x] widget 隐藏、禁用、销毁时自动释放 capture。
- [x] capture 丢失时发送 lostCapture/cancel 语义。说明：替换、隐藏、禁用、移除、销毁和 Escape 会依次发送 `XGE_EVENT_XUI_CAPTURE_LOST` 与 `XGE_EVENT_XUI_CAPTURE_CANCEL`；主动 `xgeXuiSetCapture(NULL)`、`xgeXuiSetPointerCapture(NULL)` 和 `xgeXuiReleaseWidgetCapture` 不发送 lost/cancel，避免干扰控件正常 mouse up 收尾。
- [x] Slider、SplitLayout divider、ScrollBar thumb、拖拽控件改用 pointer capture。说明：Button icon mode、CheckBox、Radio、Toggle、StatusBar、Tabs、Toolbar、Input/TextEdit selection、Slider、SplitLayout divider、SplitLayout、Window、ScrollBar、ScrollView、ListView、TreeView、TableView、PropertyGrid 等内置 capture 使用点已按事件 `iPointerId` 设置、查询和释放 capture；内部 active/selection 状态仍为单 pointer 语义，需要多指手势的控件后续单独扩展。
- [x] 增加拖拽跨控件、释放在控件外、capture 丢失回归测试。说明：已有 capture 路由、外部释放、Escape、隐藏/禁用自动释放、按 pointer id 路由、替换 lost/cancel、主动释放、多 pointer Escape、`xgeXuiReleaseWidgetCapture` 主动释放和 Slider/ScrollBar/ScrollView 控件层 capture 回归。

## 阶段 H：Widget 基础绘制

- [x] Widget 基类绘制 background fill。
- [x] Widget 基类绘制 border。
- [x] Widget 基类支持 radius。
- [x] Widget 基类支持 paintBefore/underlay hook，用于 backdrop 等必须先于基础背景且不受本体 content clip 截断的绘制。
- [x] 非 widget 重复项支持 item surface 语义。说明：Toast item 已使用 `xge_xui_style_t` 保存背景、边框、圆角和边框宽度，并通过统一 surface helper 绘制；后续列表/表格/树的行级样式可复用同一方向。
- [x] Widget 基类支持 focus ring。
- [x] Widget 基类支持 disabled/debug 视觉辅助。
- [x] 从具体控件中移除重复背景/边框样板代码。
  - [x] Panel 已迁移到 Widget 基础背景/边框绘制，Panel 自身只保留标题绘制。
  - [x] Popup 已迁移到 Widget 基础背景/边框绘制，不再挂载重复 paint callback。
  - [x] ScrollView 已迁移到 Widget 基础背景绘制，ScrollView paint 只保留滚动条绘制。
  - [x] virtualized ListView 已迁移到 Widget 基础背景绘制，virtualized ListView paint 只保留滚动条绘制。
  - [x] ListView 已迁移到 Widget 基础背景绘制，ListView paint 只保留列表项、滚动条和现有边框绘制。
  - [x] TreeView 已迁移到 Widget 基础背景绘制，TreeView paint 只保留节点行、展开器和滚动条绘制。
  - [x] TableView 已迁移到 Widget 基础背景绘制，TableView paint 只保留表头、单元格、网格线和滚动条绘制。
  - [x] PropertyGrid 已迁移到 Widget 基础背景绘制，PropertyGrid paint 只保留属性行、编辑态和滚动条绘制。
  - [x] Tabs 已迁移到 Widget 基础背景绘制，Tabs paint 只保留焦点覆盖和 tab 项绘制。
  - [x] Toolbar 已迁移到 Widget 基础背景绘制，Toolbar paint 只保留工具项、分隔符和 overflow 绘制。
  - [x] StatusBar 已迁移到 Widget 基础背景绘制，StatusBar paint 只保留顶部边线、状态项和进度绘制。
  - [x] Accordion 已迁移到 Widget 基础背景绘制，Accordion paint 只保留 header、content、边框和文本绘制。
  - [x] Window 已迁移到 Widget 基础背景绘制，Window paint 只保留外框、标题栏、按钮和 preview 绘制。
  - [x] ColorPicker 已迁移到 Widget 基础背景绘制，ColorPicker paint 只保留 swatch、field、palette、边框和文本绘制。
  - [x] TextEdit 已迁移到 Widget 基础背景绘制，TextEdit paint 只保留行号、selection、highlight、文本、光标、滚动条和边框绘制。
  - [x] Input 已迁移到 Widget 基础背景绘制。normal/disabled/error 背景状态由 Input 同步到 `pWidget->tStyle.iBackgroundColor`，Input paint 不再绘制本体背景。
  - [x] Toast 已迁移到 item surface 绘制。Toast overlay widget 本体不绘制 background/border，`backgroundColor`/`borderColor` 进入 `tItemStyle` 和每条 item 的 `tStyle`。
  - [x] Menu 已拆分 panel/background 与 frame/border 语义，菜单面板背景同步到 Popup Widget 基础背景，边框使用独立 `iBorderColor`。
  - [x] Window/MsgBox 已迁移到 Widget 基础背景/边框绘制。modal backdrop 通过 Widget `paintBefore`/underlay hook 绘制，先于面板背景且不受 Window 本体 content clip 截断。
- [x] 保持文本和图标编排在具体控件层。

## 阶段 H2：OwnerDraw 与自定义重绘

- [x] 定义 `xge_xui_paint_info_t`。说明：已包含 context、widget、role、state、ownerDrawMode、part、outerRect、borderRect、paddingRect、contentRect、style、dipScale、control、itemData、itemIndex、row、column 等信息。
- [x] 定义 OwnerDraw mode。说明：已包含 `CONTENT`、`CONTENT_AND_CHILDREN`、`FULL`；默认无 owner draw，显式启用后按 mode 替换绘制。
- [x] 增加 Widget OwnerDraw 公共 API。说明：新增 `xgeXuiWidgetSetOwnerDraw`、`xgeXuiWidgetSetOwnerDrawControl`、`xgeXuiWidgetGetOwnerDrawMode`。
- [x] 补齐公开 `paintAfter` API。说明：新增 `xgeXuiWidgetSetPaintAfter`，用于所有子树之后的装饰层，不替换默认绘制。
- [x] 调整 Widget paint pipeline。说明：`CONTENT` 替换控件默认 paint 并保留 children 和基础装饰；`CONTENT_AND_CHILDREN` 替换控件默认 paint 并跳过 children；`FULL` 只调用 owner draw 回调并跳过 Widget 基础绘制链；跳过子树绘制时会清理子树 paint dirty，避免后续刷新冒泡失效。
- [x] 提供默认 PaintInfo 状态聚合。说明：Widget 基础层提供 normal/hover/active/focus/disabled；控件层后续可叠加 checked/selected/open/readonly/error 等业务状态。
- [ ] ListView 支持 item owner draw。说明：回调获得逻辑 item index、item rect、state、文本/用户数据，控件继续负责滚动、选择、hover、focus、tooltip 和键盘导航。
- [ ] virtualized ListView / VirtualScrollViewBase 支持 item owner draw。说明：回调必须使用逻辑 item index，不暴露可复用 slot index 作为业务 index。
- [ ] TreeView 支持 node owner draw。说明：回调获得 visible index、node id/level/expanded/selected/hover/focus 状态，控件继续负责展开、选择、滚动和缩进。
- [ ] TableView 支持 header/cell owner draw。说明：回调获得 row、column、cell rect、state、排序/选中/hover 信息，控件继续负责列宽、排序、滚动和 grid line 策略。
- [ ] PropertyGrid 支持 row/value cell owner draw。说明：回调获得分类行、属性行、editor 类型、readonly/changed/error 状态，控件继续负责选择、折叠和编辑入口。
- [ ] Toolbar / StatusBar / Accordion 等复合控件评估 item/segment owner draw。说明：只为重复项或明确 part 提供重绘接口，不强行把控件内部拆成大量 widget。
- [ ] XSON owner draw 口径。说明：第一版只允许引用 C 侧注册的 owner draw 名称；不允许脚本或内嵌绘制代码；加载期固化引用，paint 热路径不得查找字符串。
- [x] 增加 Widget OwnerDraw 回归测试。说明：已覆盖默认绘制被屏蔽、基础背景/边框保留、clip 生效、children 按 mode 绘制、FULL 跳过基础链，以及 `CONTENT_AND_CHILDREN` / `FULL` 跳过子树时清理 child paint dirty。
- [ ] 增加复合控件 OwnerDraw 回归测试。说明：List/virtualized ListView/Table 逻辑 index 等复合控件测试待 item/cell owner draw 接入后补齐。
- [x] 更新公开 guide。说明：新增 `docs/guide/xui-widget-ownerdraw-intro.md`，说明 `SetPaint`、`paintBefore/After`、Widget OwnerDraw、控件 item owner draw 的边界和推荐用法。

## 阶段 I：ScrollViewBase

- [x] 定义 ScrollViewBase 数据结构。
- [x] 支持 viewport/content rect。
- [x] 支持 scrollX/scrollY、contentWidth/contentHeight。
- [x] 支持 scrollTo、scrollBy、ensureVisible。说明：`SetOffset` 为 scrollTo 语义，`ScrollBy` 与 `EnsureRectVisible`/`EnsureChildVisible` 已落地。
- [x] 支持 wheelAxis：vertical/y、horizontal/x、both，默认 vertical。
- [x] 支持 dragMode/contentDrag：none 默认关闭，content/pan/drag 显式开启。
- [x] 支持 scrollbarDrag，默认 true。
- [x] 支持 nestedScroll：consume、passEdge。
- [x] 支持 content clip 和滚动条绘制。
- [x] ScrollView 控件迁移到 ScrollViewBase。
- [x] 增加横向、纵向、双向、嵌套滚动、左键不抢占回归测试。

## 阶段 J：VirtualScrollViewBase

- [x] 定义 VirtualScrollViewBase 数据结构。
- [x] 支持 item count。
- [x] 支持固定 item height 第一版。
- [x] 支持 visible range 计算。
- [x] 支持 slot create / bind / recycle。
- [x] 支持 ensureIndexVisible。
- [x] 支持 selected/hover/focus index 与 slot 解耦。说明：VirtualScrollViewBase 维护 `iSelected`、`iHover`、`iFocus` 逻辑 item index，并提供对应 setter/getter；ListView、TreeView、TableView、PropertyGrid 已同步到 `tBase`，slot recycle、visible window 和控件私有稳定 id 不再影响这三类 index。
- [x] ListView 迁移到 VirtualScrollViewBase。说明：ListView 已内嵌 `tBase` 复用滚动、选择、hover/focus index、滚动条状态，文本、禁用项、多选和行渲染仍保留在控件层。
- [x] TreeView 迁移到 VirtualScrollViewBase。说明：TreeView 已内嵌 `tBase` 复用滚动、选中/悬停/焦点 visible index、滚动条状态和 capture；节点数组、展开后的可见节点数组、active visible、稳定 selected node id 和节点渲染仍保留在控件层。
- [x] TableView 迁移到 VirtualScrollViewBase。说明：TableView 已内嵌 `tBase` 复用 row count、selected/hover/focus row、row height、垂直滚动、滚动条状态和 thumb capture；列定义、列宽 resize、排序、cell adapter 和表格渲染仍保留在控件层。
- [x] PropertyGrid 迁移到 VirtualScrollViewBase。说明：PropertyGrid 已内嵌 `tBase` 复用可见 property row count、selected/hover/focus visible index、row height、垂直滚动、滚动条状态和 thumb capture；分类/属性数组、稳定 selected item index、编辑状态、change 回调和属性行渲染仍保留在控件层。
- [x] 增加大量 item、slot 复用、滚动保持选择项可见回归测试。

## 阶段 K：XSON 同步

- [x] XSON 建立 type -> role 映射。
- [x] Control 默认拒绝普通 children。
- [x] Container 允许 children。
- [x] Viewport 允许 children 并启用滚动裁剪。
- [x] virtualized ListView 只允许 itemTemplate。
- [x] Overlay 使用 layer / portal / owner 进入 overlay root。说明：基础 OverlayManager API 已支持 layer、owner 和 overlay root 挂载；XSON `popup`、`Window`、`MsgBox` 声明节点通过 portal 进入 overlay root，并在 `PageSyncStyle`/`PageUnload` 中按 path 注册表刷新和释放；Menu 的实际浮层通过 MenuInit 接入 overlay root，widget tooltip 的实际浮层通过 context 内部提示服务接入 overlay root；XSON 不再支持独立 `type:"tooltip"` 节点。
- [x] 支持字段：overflow、zIndex、layer、tabStop、tabIndex、imeMode、hitTestVisible、inputTransparent、borderColor、borderWidth、focusRingColor、focusRingWidth、disabledOverlay、debugOutlineColor、debugOutlineWidth。
- [x] 支持 ScrollViewBase 字段：wheelAxis、dragMode、scrollbarDrag、nestedScroll。
- [x] 错误信息带字段路径和 role 规则。
- [x] 更新 XSON 示例和 guide。说明：guide、基础 XSON 字段回归和聚合控件示例已同步；`xui_xson_controls_lab` 覆盖基础/选择/范围/ColorPicker/DatePicker/数据/体验控件和 tooltip。

## 阶段 L：控件迁移与成熟度重验

- [x] Button（含图标模式）、Label、Image 重验裁剪、状态、事件、焦点。说明：四者均固定为 Control role 默认 `overflow: clip`；Button icon mode 覆盖 focus in/out、鼠标/键盘事件、默认裁剪和 `overflow: visible` 放开裁剪；Label/Image 覆盖默认 Control 裁剪、不默认焦点和文本/图片裁剪路径。
- [x] Input、TextEdit 重验 IME、selection、文字裁剪、焦点。说明：两者均固定 Control role 默认 `overflow: clip`、focusable 和 IME enabled；focused readonly 会主动切到 IME disabled，readonly 解除后恢复 IME；Input 覆盖 password/readonly/disabled IME、selection 替换、鼠标拖选、双击选词、IME composition/candidate；TextEdit 覆盖 readonly IME、selection、键盘/鼠标选择、undo/redo、滚动、word wrap、candidate 和文本裁剪路径。
- [x] CheckBox、Radio、Toggle、Slider、Progress 重验事件和 pointer capture。说明：CheckBox、Radio、Toggle 固定 Control role、默认 `overflow: clip`、IME disabled、focusable、鼠标/键盘切换、外部松开不触发变更、Escape capture lost/cancel 清 active；Slider 固定 Control role、focus/capture、拖拽、外部释放、Escape capture lost/cancel，并补齐键盘 Home/End/方向键/Page 调值；Progress 固定为只读 Control，不 focus、不注册事件、不抢 capture。
- [x] ScrollBar、ScrollView 重验 ScrollViewBase。说明：ScrollView 与 ScrollViewBase 已重验；独立 ScrollBar 固定 Control role、默认 `overflow: clip`、IME disabled、focusable，补齐按钮/轨道/thumb 按压 capture、Escape capture lost/cancel、横纵向拖拽、滚轮、键盘 Home/End/方向键/Page 调值和 disabled 回归。
- [x] ListView 重验 VirtualScrollViewBase。说明：virtualized ListView 直接等同于 VirtualScrollViewBase；ListView 已通过 `tBase` 重验通用滚动、选择、捕捉和 viewport 口径。
- [x] Popup、Menu、ComboBox、Tooltip、Window 重验 overlay layer/top 和 focus restore。说明：Popup/Window 固定 Overlay role、layer 与 IME disabled，补齐自动/显式 focus restore、Escape/outside close 和重复打开不过期恢复；Menu/ComboBox 通过 Popup overlay 恢复 owner/控件焦点并覆盖 overlay top；Tooltip 已作为 context 内部 overlay popup 覆盖 tooltip layer/top；完整 OverlayManager 统一接管仍由阶段 D 追踪。
- [x] TreeView、TableView、PropertyGrid 重验 Widget 适配。说明：TreeView、TableView、PropertyGrid 已通过 `tBase` 重验通用滚动、选择镜像、捕捉和 viewport 口径。
- [x] ColorPicker 补齐完整基础控件能力并重验。说明：ColorPicker 已支持 palette 点击选择、RGBA 数字字段键盘编辑、hex 字段键盘编辑、Enter 提交、Escape 取消、无效输入错误态、focus out 提交/取消和编辑态 IME 申请策略。
- [x] DatePicker 实现并纳入基础控件验收。说明：DatePicker 已支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 `examples/xui_date_picker_lab` 验证。
- [x] Toggle、NumericInput、Separator、SplitLayout divider 重验 Widget 适配。说明：这些单 widget 控件已覆盖 role、默认裁剪、事件/焦点/capture、IME 策略、状态和 Unit 清理；搜索框已清理为非 core 组合控件口径。
- [x] Tabs、Toolbar、StatusBar、Accordion、Toast 重验复合控件口径。说明：已覆盖 role、布局/绘制、选择/点击回调、tooltip/overflow/队列等控件私有状态，并通过聚合 XSON 示例覆盖声明式入口。
- [x] Panel、Window、SplitLayout、MsgBox 重验容器/窗口/组合布局口径。说明：Panel/Window/MsgBox 已覆盖基础绘制、overlay/focus/关闭策略；SplitLayout 已覆盖 Container role、pane/divider 布局、divider capture、shadow drag 和 Unit 清理。
- [x] RichTextView、CodeEditor、NodeGraph、Timeline 建立可选高级组件路线，不阻塞核心。说明：这些控件按可选高级组件推进，不进入 Widget 基础层和基础应用控件成熟度阻塞项；核心侧仅保留 ScrollViewBase、VirtualScrollViewBase、文本输入、选择、焦点、clip 和绘制基础设施要求。

## 阻塞与决策记录

- [x] 决策：统一术语使用 Widget，不再引入 Element。
- [x] 决策：保留一个底层 Widget 类型，通过 role 区分 Control、Container、Viewport、Overlay。
- [x] 决策：ScrollViewBase 和 VirtualScrollViewBase 分成两层，不做单个大基类。
- [x] 决策：ScrollViewBase 内容拖拽默认关闭，避免干扰地图/画布/编辑器控件。
- [x] 决策：Widget 基类只绘制背景、边框、focus ring，不封装文本和图标编排。
- [x] 决策：IME 默认禁用，文本控件显式申请。
- [x] 决策：代码里继续使用 `widget`、`tRect` 等简洁兼容命名；`tRect` / `xgeXuiWidgetGetRect` 作为 borderRect 兼容入口，不在代码命名里加入版本号。
- [x] 决策：Widget 基础设施负责低层语义事件，控件业务事件继续留在具体控件层；Widget 不理解 checked、selected、value、open 等控件业务状态。
- [x] 决策：MouseMove、hover、tooltip、hotkey 等高频路径必须 opt-in，通过 eventMask/subtreeEventMask 或专用注册表避免无人关心时仍做 hit test、树扫描或回调构造。
- [x] 决策：控件迁移不随阶段 E2 一次性完成；每个控件后续按独立重构方案接入新的 Widget 语义事件。
