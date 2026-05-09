# XUI Widget V2 基础设计

本文档是 2026-05-07 起 XGE 内置 XUI 基础层重构的权威设计文档。旧文档中与本文冲突的口径，以本文为准。

## 1. 背景

当前 XUI 已有 `xge_xui_widget_t`、布局、XSON、控件和浮层能力，但基础层仍不够成熟：

- clip 分散在控件里，文字和子树容易渲染溢出本体。
- Z 序依赖局部 `z/order` 和补丁式 overlay base，绘制、hit test、事件目标不够统一。
- 事件分发缺少明确的 tunnel、target、bubble 契约。
- focus、TAB、IME、pointer capture、scroll 等策略分散在控件实现里。
- 基础控件没有统一盒模型、背景、边框和 PaintContext。
- ScrollView、ListView、VirtualList、TreeView、TableView、PropertyGrid 等滚动类控件缺少共同基础设施。

本轮重构不背负旧行为兼容包袱。示例、XSON 页面和相关程序可以随新基础层一起重构。

## 2. 目标

- 建立可靠的 Widget 基类，让控件不再各自重复实现通用行为。
- 统一布局、clip、Z、事件、焦点、TAB、pointer capture、IME、box model 和基础绘制。
- 将 Widget 事件系统升级到接近 VB6 易用度，并保留 DirectUI 所需的高性能热路径：常用鼠标、键盘、焦点、捕获、点击、双击、上下文菜单、拖拽、热键和命令事件由基础设施统一合成和分发。
- 建立 ScrollViewBase 与 VirtualScrollViewBase，作为滚动类控件基础设施。
- 明确 XSON 的 `type -> Widget role`、children、overflow、scroll 和 IME 口径。
- 先把现有控件做到任意常见场景高可用；ColorPicker 与 DatePicker 已按基础控件口径补齐并重验。

## 3. 非目标

- 不实现浏览器 DOM、CSS cascade、完整 Flexbox、完整 CSS Grid。
- 不把 RichTextView、CodeEditor、NodeGraph、Timeline 放进核心基础层。
- 不在 Widget 基类里封装文本排版和图标编排。文本、图标、slot 编排属于具体控件。
- 不让普通 `overflow: scroll` 隐式创建复杂滚动容器；需要滚动时使用 ScrollViewBase 派生控件。

## 4. 术语

- Widget：XUI 树节点和所有控件的基础对象。
- Control：普通控件，默认不承载用户 children，例如 Button、Label、Input、Image。
- Container：布局容器，允许 children 并参与子布局，例如 Panel、Row、Column、Grid、Dock。
- Viewport：有裁剪和滚动语义的容器，例如 ScrollView。
- Overlay：脱离普通父子绘制层级的浮层，例如 Popup、Menu、Tooltip、Dialog、Window。

一个底层类型仍然是 `xge_xui_widget_t`，但通过 role 区分行为：

```c
XGE_XUI_WIDGET_ROLE_CONTROL
XGE_XUI_WIDGET_ROLE_CONTAINER
XGE_XUI_WIDGET_ROLE_VIEWPORT
XGE_XUI_WIDGET_ROLE_OVERLAY
```

## 5. Widget 基类职责

Widget 基类必须负责：

- 布局系统：measure、arrange、dirty layout、content size、min/max、margin、padding。
- 事件输入：hit test、tunnel、target、bubble、代理转发、拦截、消费，以及基础语义事件合成。
- 事件兴趣：每个 widget 维护自身 eventMask，context 维护 subtreeEventMask，用于跳过无人关心的 MouseMove、hover、tooltip、hotkey 等热路径工作。
- 焦点系统：focusable、focused、focus scope、focus restore。
- TAB 顺序：tabStop、tabIndex、容器内遍历和 overlay/modal 限制。
- Z 序：layer、zIndex、treeOrder 的统一排序。
- pointer capture：鼠标/触摸独占捕获，用于 drag、slider、splitter 等。
- IME：控件是否申请 IME；默认非文本控件禁用 IME。
- tooltip：每个 widget 可设置静态 tooltip、清空为 NULL，或提供动态 resolver 按命中状态返回不同文本/自绘提示。
- 盒模型：margin、border、padding、content rect。
- 基础绘制：背景、边框、圆角、focus ring、disabled overlay、debug outline。

Widget 基类不负责：

- 文本布局、文本裁剪策略以外的高级排版。
- 图标和文本组合布局。
- 控件业务状态，例如 checked、selected、open、value。
- 复杂数据绑定和虚拟数据 adapter。

## 6. 盒模型

Widget 使用固定盒模型：

```text
outerRect
  -> margin
  -> borderRect
  -> border
  -> paddingRect
  -> contentRect
```

- layout 使用 outerRect 分配空间。
- 背景绘制到 borderRect；borderWidth 参与 paddingRect/contentRect 计算。
- 子布局默认发生在 contentRect。
- hit test 默认使用 borderRect。
- overflow、clip、ScrollViewBase viewport 使用 contentRect 作为默认裁剪边界。
- 当前代码中旧字段/API `tRect`、`xgeXuiWidgetGetRect` 作为兼容入口继续表示 borderRect，不引入带版本号的代码命名。
- 通用 borderColor、borderWidth、focusRingColor、focusRingWidth、disabledOverlay、debugOutlineColor、debugOutlineWidth 已进入基础 style；未设置 borderWidth 时 paddingRect 与 borderRect 相同。

## 7. Overflow 与 Clip

Widget 支持：

- `visible`：允许绘制溢出，主要用于特殊装饰或调试。
- `hidden`：裁剪到 widget 内容区域，不产生滚动。
- `clip`：显式裁剪到 contentRect。
- `scroll`：只作为声明语义，不自动生成完整滚动控件；实际滚动使用 ScrollViewBase。

普通 Control 默认应裁剪自身文本和内容，不允许文字溢出控件本体。Container 默认策略由具体容器选择，但生产控件不应依赖子节点无限溢出来显示核心内容。

PaintContext 维护 clip stack：

1. 调用 Widget `paintBefore`/underlay 回调，用于 Dialog backdrop 等必须位于基础背景之前、且不受本体 content clip 截断的绘制。
2. 绘制 Widget 基础背景，受父级 clip 约束，但不被自身 content clip 截断。
3. 按 overflow / viewport 推入 content clip。
4. 调用控件 paint，并按 Z 序绘制 children。
5. 离开 widget 时弹出自身 content clip。
6. 绘制 Widget disabled overlay、基础边框、focus ring 和 debug outline，仍受父级 clip 约束。

Toast item、列表行、表格行这类非 widget 的重复项不强行拆成子 widget。它们使用 item surface 语义：以 `xge_xui_style_t` 保存背景、边框、圆角和边框宽度，由统一 surface helper 绘制；文本、图标和业务状态仍留在具体控件层。

## 8. Z 序与 Layer

绘制、hit test 和事件目标选择必须使用同一套排序：

```text
layer > zIndex > treeOrder
```

建议 layer：

- Normal：普通树。
- Floating：局部浮动元素。
- Popup：Popup、Menu、ComboBox 下拉层。
- Modal：Dialog、modal Window。
- Tooltip：Tooltip。
- DragAdorner：拖拽预览。
- Debug：调试 overlay。

`treeOrder` 由父容器 children 顺序决定。禁止控件内部自行维护一套与全局排序冲突的 overlay base。

Tooltip 是 widget 内部机制，不是公开控件实例。context 负责命中检测、延迟、定位、screen clamp、overlay 绘制和关闭；具体 widget 只提供静态描述或动态 resolver。动态 resolver 用于 Toolbar item、ListView item、TableView cell 等同一 widget 内部不同区域显示不同提示的场景，返回空表示当前命中区域不显示 tooltip。

## 9. 事件路由与语义事件

### 9.1 原始事件路由

原始 XGE 输入事件仍然是所有 Widget 事件的基础，分发流程固定为：

1. 如果存在 pointer capture，事件优先送到 capture widget。
2. 否则按 layer/zIndex/treeOrder 做 hit test，找到 target。
3. 从 root 到 target 执行 tunnel。
4. 在 target 执行 target handler。
5. 如果未消费，从 target 父级向 root 执行 bubble。
6. 如果没有 widget 消费事件，则返回给 XGE 处理。

事件处理结果：

- continue：未处理，继续路由。
- handled：已处理，但允许某些后续阶段继续；最终阻止 XGE fallback。
- consumed：已消费，停止路由。

控件可以代理转发事件，但必须保留 originalTarget、currentTarget、phase、pointer id、capture 和时间戳信息，避免调试和行为判断失真。

### 9.2 Widget 基础语义事件

Widget 基础层在原始事件之上合成一组稳定语义事件。它们是控件开发的基础设施，不等同于 Button 的 `Click`、Input 的 `Change`、ListView 的 `Select` 等控件业务事件。

鼠标与指针事件：

- `MouseEnter` / `MouseLeave`：鼠标进入或离开 widget hover path。必须支持父容器路径变化，不只支持单个 hover widget。
- `MouseMove`：仅当 widget 或子树注册兴趣、存在 capture、拖拽、tooltip、cursor、drop target 等需求时才命中和分发。
- `MouseDown` / `MouseUp`：携带 button、pointer id、坐标、modifier、click count。
- `MouseWheel`：携带 deltaX/deltaY 和 wheel axis，支持 ScrollViewBase nested scroll 上浮。
- `Click`：同一 pointer 在同一 widget 或其有效点击区域内按下并释放，且未超过拖拽阈值。
- `DoubleClick`：同一 button、同一 target、同一点击区域在系统时间/距离阈值内第二次点击成立。第二次点击仍可触发 `Click`，同时额外触发 `DoubleClick`；需要抑制第二次 `Click` 的控件可在控件层自行处理。
- `ContextMenu`：右键点击、长按或键盘上下文键/Shift+F10 统一合成。

当前基础层已实现 left click、double click、右键 context menu、长按 context menu、Menu 键和 Shift+F10 合成。Click/DoubleClick 使用 6px 移动距离阈值，DoubleClick 使用 0.50 秒时间阈值；隐藏、禁用、移除或销毁命中的 widget 会清理 click 状态。长按触发时会先发送 context begin，再合成 ContextMenu，并在释放或取消时发送 context end/cancel。

键盘与文本事件：

- `KeyDown` / `KeyUp`：物理按键、modifier、repeat 信息。
- `TextInput`：文本输入字符，与 `KeyDown` 分离，IME composition 提交后进入该路径。
- `HotKey`：由 context/focus scope 注册表匹配，不在每次按键时全树扫描。注册项按 key+modifier 直接匹配，触发前必须位于当前 modal/focus scope 内，并且目标 widget 及祖先处于 visible/enabled 状态；隐藏、禁用或脱离当前树的 widget 不触发热键。
- `Command`：热键、菜单、工具栏、默认按钮等统一触发的命令语义，用于把多个入口汇聚到同一业务动作。Command 事件通过 `xge_xui_command_t` 携带数字 command id、字符串 command name、source widget 和业务数据；Widget 基础层只负责路由，不解释 checked、selected、value、open 等控件业务状态。

键盘分层使用现有 XGE 基础事件作为 Widget 语义入口：`XGE_EVENT_KEY_DOWN` / `XGE_EVENT_KEY_UP` 承载物理按键和 modifier，`XGE_EVENT_TEXT` 承载文本 codepoint；IME composition 提交最终进入文本输入路径。Widget 的 keyboard mask 只做兴趣聚合，不能把物理按键当文本输入，也不能用 `TextInput` 处理快捷键。

焦点、捕获和拖拽事件：

- `GotFocus` / `LostFocus`：焦点变化的语义事件，和原始 `XUI_FOCUS_IN/OUT` 保持一致。
- `CaptureLost` / `CaptureCancel`：pointer capture 异常丢失或被取消。
- `DragBegin` / `DragMove` / `DragEnd` / `DragCancel`：基础拖拽手势，只在 widget 显式注册或调用 drag API 时启用，不默认抢占普通 MouseMove。基础层在按下后超过 6px 阈值才触发 DragBegin，并通过 pointer capture 接管后续 Move/End/Cancel；小幅移动仍保留普通点击路径。
- `BoundsChanged` / `VisibleChanged` / `EnabledChanged`：基础状态变化通知，供 overlay、tooltip、capture、focus restore 和调试工具维护缓存。

状态变化通知为 widget-local target 事件，不做默认冒泡。同值设置不触发；只有注册了对应事件兴趣的 widget 才会收到通知。`BoundsChanged` 携带新 rect 坐标和宽高变化量，`VisibleChanged` / `EnabledChanged` 通过 `iParam1/iParam2` 携带新旧布尔值。

隐藏、禁用、移除和销毁 widget 时，基础层必须同步清理所有指向该 widget 或其子树的 context 引用，包括 capture、focus、hover path、tooltip owner、context press、click press、drag state 和 last click target。移除或销毁 widget 时还必须清理该子树的 HotKey/Command 注册表项，并刷新父级 subtreeEventMask。清理必须发生在状态变更生效时，避免后续事件命中已不可交互的节点。

### 9.3 事件注册与性能口径

Widget 事件必须是 opt-in。没有注册回调、没有事件掩码、没有相关基础设施需求时，不应为该 widget 或其子树执行热路径工作。

- 每个 widget 维护 `eventMask`，表示自身关心的语义事件。
- 每个 widget 维护 `subtreeEventMask`，表示自身子树中是否有人关心某类事件；子节点 mask 变化时向祖先增量传播。
- `MouseMove` 默认不做完整 hit test；仅当 hover、move、tooltip、cursor、drop target、drag 或 capture 至少一项存在时才进入命中路径。
- Click/DoubleClick 状态只在子树声明 click 或 double click 兴趣后启用；移动阈值记录只依赖按下后的坐标差，不让普通 `MouseMove` 因点击状态而扫描整棵树。
- Drag 状态只在 widget 显式 `dragEnabled` 或注册 Drag 事件时启用；未声明 drag interest 的子树不会因拖拽基础设施增加 MouseMove 成本。
- 连续 `MouseMove` / `TouchMove` 在事件队列层做队尾合并：只有同类型、同 pointer、同 modifier/button 状态的连续 move 才合并；合并后保留最后坐标并累加 delta，不跨越 down/up/key/text 等边界事件，也不合并不同 pointer。
- HotKey 走 context/focus scope 注册表，按 key+modifier 直接查找，再按 modal/focus scope 过滤；禁止按键时遍历整棵 widget tree。注册和注销会维护 hotkey event interest，普通 widget 未注册热键时不进入 hotkey 回调路径。
- Tooltip 是 Widget 内部机制。设置 tooltip 或动态 resolver 会让 widget 自动进入 hover/tooltip interest；tooltip 为 NULL 时不显示也不保留额外 hover 成本。
- Tooltip owner 隐藏、禁用、移除或销毁时，Widget 基础层必须同步关闭 tooltip popup，并清除 overlay owner 引用；不能依赖下一帧 update 兜底清理。
- Debug/trace 可记录事件路由、mask 命中、drag state、HotKey/Command 匹配结果，但必须受 debug 开关保护，不进入 release 热路径。当前由 `xgedbgXuiEventTrace` 提供 debug-only 预览，不实际派发事件，不改变运行状态。

### 9.4 控件接入口径

本阶段只升级 Widget 和基础设施层。具体控件后续逐个重构时再接入新的语义事件，避免一次性改动所有控件造成行为混乱。

控件业务事件继续按控件语义命名：

- Button/IconButton：`Click`、`DefaultAction`。
- Input/TextEdit：`Change`、`Changing`、`Submit`、`Cancel`。
- List/Tree/Table/PropertyGrid：`Select`、`Activate`、`ContextMenu`。
- Popup/Menu/Dialog：`Open`、`Close`、`Cancel`、`Command`。

控件业务事件可以由 Widget 基础语义事件触发，但不得反向污染 Widget 基类，让基类知道具体控件的 checked、selected、value、open 等业务状态。

## 10. 焦点与 TAB

Widget 基础层维护单一 focused widget，并支持 focus scope：

- `focusable`：是否可获取焦点。
- `tabStop`：是否进入 TAB 顺序。
- `tabIndex`：同一 scope 内显式排序；未设置时按 treeOrder。
- `defaultAction`：Enter 触发默认动作。
- `cancelAction`：Escape 触发取消动作。
- `focusRestore`：Popup/Dialog 关闭后恢复 owner 焦点。

Overlay 和 modal 会限制 TAB 搜索范围，避免焦点跳出当前交互上下文。

## 11. Pointer Capture

Widget 基础层提供 pointer capture：

- capture 后，同一 pointer id 的 move/up/cancel 继续发送给 capture widget。
- capture widget 隐藏、禁用、销毁或失去上下文时必须自动释放。
- capture 丢失时必须依次发送 `XGE_EVENT_XUI_CAPTURE_LOST` 与 `XGE_EVENT_XUI_CAPTURE_CANCEL`，控件清理 active/drag 状态；主动释放 capture 不发送 lost/cancel。
- Slider、Splitter、ScrollBar thumb、拖拽排序等控件必须使用 pointer capture。

## 12. IME 策略

IME 使用激进口径，适配游戏场景：

- 默认 `imeMode=disabled`，非文本控件获得焦点也不启用 IME。
- 文本输入控件使用 `imeMode=enabled` 或 `auto` 申请 IME。
- Password 默认禁用 IME，除非控件显式允许。
- focus 变化时由 FocusManager 统一调用 XGE IME 开关和候选框位置更新。

XSON 字段：

```json
{ "imeMode": "disabled" }
{ "imeMode": "enabled" }
{ "imeMode": "auto" }
```

## 13. ScrollViewBase

ScrollViewBase 是普通滚动视图基础层，负责：

- viewport/content rect。
- scrollX/scrollY、contentWidth/contentHeight。
- 滚动边界、clamp、scrollTo、scrollBy、ensureVisible。
- content clip。
- 滚动条几何、绘制和 thumb hit test。
- wheel、scrollbar drag、nested scroll。

策略字段：

- `wheelAxis`：`none`、`x`、`y`、`both`，默认 `y`。
- `dragMode`：`none`、`middle`、`touch`、`left`、`custom`，默认 `none`。
- `scrollbarDrag`：默认 `true`。
- `nestedScroll`：默认 `bubbleWhenCannotScroll`。

左键内容拖拽不能默认开启。地图编辑器、画布编辑器、节点编辑器等控件需要完整左键、右键和拖拽语义，基础层默认抢占鼠标会降低可组合性。

代码口径使用 `xge_xui_scroll_view_base_t` / `xge_xui_scroll_view_base`，不在类型名里写版本号。第一阶段 ScrollView 直接等同于 ScrollViewBase：`xge_xui_scroll_view_t` 是 base 类型别名，现有 `xgeXuiScrollView*` API 继续保留，同时新增 `xgeXuiScrollViewBase*` API 给后续复用控件使用。`xgeXuiScrollViewSetOffset` 对应 scrollTo 语义，`xgeXuiScrollViewScrollBy` / `xgeXuiScrollViewBaseScrollBy` 提供相对滚动。

## 14. VirtualScrollViewBase

VirtualScrollViewBase 继承 ScrollViewBase，负责虚拟化：

- item count。
- item extent：固定高度第一版，后续支持可变高度缓存。
- visible range 计算。
- slot create / bind / recycle。
- selected/hover/focus index 与滚动位置、slot recycle 解耦。
- ensureIndexVisible。

代码口径使用 `xge_xui_virtual_scroll_view_base_t` / `xge_xui_virtual_scroll_view_base`。第一阶段 VirtualList 直接等同于 VirtualScrollViewBase：`xge_xui_virtual_list_t` 是 base 类型别名，现有 `xgeXuiVirtualList*` API 继续保留，同时新增 `xgeXuiVirtualScrollViewBase*` API 给后续 ListView、TreeView、TableView、PropertyGrid 等控件复用。base 层使用通用 `xge_xui_virtual_scroll_*_proc` 回调类型，旧 `xge_xui_virtual_list_*_proc` 是兼容别名。`iSelected`、`iHover`、`iFocus` 均表示逻辑 item index，不表示可复用 slot index。

ListView 已内嵌 `tBase` 复用 VirtualScrollViewBase 的滚动、选择、hover/focus index 和滚动条状态。ListView 的文本、禁用项、多选、item renderer 和行背景仍属于控件层；它不为每一行创建 widget slot，也不复用 base 的 slot layout。

TreeView 已内嵌 `tBase` 复用 VirtualScrollViewBase 的滚动、selected/hover/focus visible index、滚动条和捕捉状态。TreeView 自己保留节点数组、展开后的可见节点数组、稳定 selected node id、active visible 和节点渲染；`tBase.iItemCount` 镜像展开后的可见节点总数，`tBase.iSelected` 镜像当前 selected visible index。这样避免把 TreeView 的稳定 node id 和 base 的 index 语义混在一起。

TableView 已内嵌 `tBase` 复用 VirtualScrollViewBase 的 row count、selected/hover/focus row、row height、垂直滚动、滚动条和 thumb capture 状态。TableView 自己保留列定义、列宽 resize、排序、cell adapter、active row 和表格渲染；`tBase.iItemCount` 是当前 row count，`tBase.iSelected` 是当前 selected row。横向列 resize 不进入 base，避免把表格特有的列交互污染到通用虚拟滚动层。

PropertyGrid 已内嵌 `tBase` 复用 VirtualScrollViewBase 的可见 property row count、selected/hover/focus visible index、row height、垂直滚动、滚动条和 thumb capture 状态。PropertyGrid 自己保留分类/属性数组、展开后的可见项数组、稳定 selected item index、编辑态、change 回调和属性行渲染；`tBase.iItemCount` 镜像当前可见 property row 总数，`tBase.iSelected` 镜像 selected item 在可见数组中的 index。这样避免把 PropertyGrid 的稳定 item index 和 base 的 visible index 语义混在一起。

## 15. XSON 影响

XSON 必须同步 Widget V2：

- `type` 映射到 role：Control、Container、Viewport、Overlay。
- Control 默认拒绝普通 `children`。
- Container 允许 `children`。
- `virtualList` 拒绝普通 `children`，只允许 `itemTemplate`。
- 新增或统一字段：`overflow`、`zIndex`、`layer`、`tabStop`、`tabIndex`、`imeMode`、`hitTestVisible`、`inputTransparent`、`borderColor`、`borderWidth`、`focusRingColor`、`focusRingWidth`、`disabledOverlay`、`debugOutlineColor`、`debugOutlineWidth`。
- ScrollViewBase 字段：`wheelAxis`、`dragMode`、`scrollbarDrag`、`nestedScroll`。
- 加载错误必须带字段路径，例如 `tree.children[3].children not allowed on control button`。

## 16. 控件成熟度口径

现有控件必须先迁移到 Widget V2，再判定成熟：

- Button、IconButton、Label、Image、Input、TextEdit、CheckBox、Radio、Switch、Slider、Progress 等基础控件：先重验裁剪、事件、焦点和 IME。
- ScrollView、ListView、TreeView、TableView、PropertyGrid：迁移到 ScrollViewBase / VirtualScrollViewBase。
- Popup、Menu、ComboBox、Tooltip、Dialog：迁移到 Overlay layer 与 focus restore。
- ColorPicker：已补齐完整基础控件能力并重验，支持 palette、RGBA/hex 字段编辑、错误态和编辑态 IME 策略。
- DatePicker：已作为基础应用控件补齐，支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 lab 验证。
- RichTextView、CodeEditor、NodeGraph、Timeline：作为可选高级组件，核心基础层稳定后再推进。

## 17. 调试与验证

调试能力应能显示：

- widget role、id、type。
- outerRect、borderRect、paddingRect、contentRect。
- overflow、clip stack。
- layer、zIndex、treeOrder。
- hit test target、event route phase。
- focus、tab order、pointer capture、imeMode。
- ScrollViewBase viewport、content size、offset、visible range。

这些能力进入 `xgedbg`，由 `XGE_DEBUGMODE` 隔离。
