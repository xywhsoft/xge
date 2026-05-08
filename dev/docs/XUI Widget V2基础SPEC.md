# XUI Widget V2 基础 SPEC

本文档跟踪 2026-05-07 起 XGE 内置 XUI Widget V2 基础层重构进度。旧 SPEC 保留历史记录；涉及 clip、Z、事件、焦点、滚动、IME、box model 和控件成熟度的验收，以本文为准。

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
- [x] Pointer capture 支持 drag、lost capture 和控件销毁/隐藏/禁用自动释放。说明：pointer 0 兼容 capture、按 pointer id capture 表、move/up/cancel 优先送达、lost/cancel 通知、隐藏/禁用/移除/销毁自动释放已落地；内置拖拽/按压控件已按事件 `iPointerId` 设置、查询和释放 capture。
- [x] FocusManager 统一管理焦点、TAB、overlay focus scope、focus restore 和 IME。说明：单一 focus、`focusable`、`tabStop`、`tabIndex`、TAB 遍历、overlay/modal TAB scope、命名 focus scope、Enter default action、Escape cancel action、`imeMode` 焦点同步、候选框位置缓存和 Popup/Dialog focus restore 已落地；Input/TextEdit 通过 widget 候选框解析器接入，普通 widget 默认使用 content rect。
- [x] ScrollViewBase / VirtualScrollViewBase 被 ScrollView、VirtualList、ListView、TreeView、TableView、PropertyGrid 等控件复用。说明：ScrollView 已直接复用 ScrollViewBase，VirtualList 已直接复用 VirtualScrollViewBase，ListView、TreeView、TableView、PropertyGrid 已内嵌 `tBase` 复用 VirtualScrollViewBase 通用状态。
- [x] XSON 同步 Widget role、children、overflow、z、scroll、IME 字段和错误路径。说明：type -> role、Control children 拒绝、Viewport/VirtualList children 规则、overflow/z/layer/focus/IME/ScrollViewBase 字段和错误路径均已进入回归。
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
- [x] 为 Overlay 接入 overlay root / layer 语义。说明：overlay root、popup、dialog、toast 等已标记为 Overlay，统一通过 `layer > zIndex > treeOrder` 排序，并由 OverlayManager 挂载到独立 overlay root。
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
- [x] OverlayManager 统一 Popup、Menu、ComboBox、Tooltip、Dialog。说明：已新增 `xgeXuiOverlayAttach` / `Detach` / `BringToFront` / `GetOwner` / `Top`，统一 overlay root 挂载、owner 元数据、layer 排序和同层 treeOrder 提升；Tooltip、Menu、ComboBox 已走统一挂载，Popup/Dialog 打开时接入 bring-to-front；XSON `popup`/`dialog`/`messageBox` 声明节点已通过 portal 进入 overlay root，Menu/Tooltip 的实际浮层分别通过内部 Popup/context tooltip service 接入 overlay root。
- [x] 移除控件内部手写 z base / 补丁式遮盖逻辑。说明：Popup/Dialog/Menu/ComboBox/Tooltip 不再维护独立 z base，默认排序由 `layer > zIndex > treeOrder` 接管；手动同层排序统一使用 `xgeXuiWidgetSetZ`。
- [x] 增加 overlay 叠放、modal、tooltip、debug layer 回归测试。说明：layer 排序、XSON layer/z、Popup/Menu/ComboBox overlay top、Dialog modal 独占、Tooltip layer、OverlayManager owner 清理、XSON portal 卸载、overlay clip stack 和 debug layer paint/hit/event target 已覆盖。

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

## 阶段 F：Focus、TAB 与 IME

- [x] FocusManager 统一 focused widget。说明：context 已维护单一 focus，隐藏/禁用会释放焦点；TAB 和 default/cancel action 已接入 overlay/modal focus scope 与命名 focus scope；IME 开关和候选框位置随焦点统一刷新。
- [x] 支持 focusable、tabStop、tabIndex。
- [x] 支持 focus scope。说明：TAB 已支持可见 modal overlay 限制到最上层 modal 子树；非 modal overlay 在当前焦点位于该 overlay 子树时限制在该 overlay 内；普通 widget 可通过 `xgeXuiWidgetSetFocusScope` 声明命名 focus scope，当前焦点位于该 scope 子树时，TAB 与 default/cancel action 限制在最近可见启用 scope 内。
- [x] 支持 Enter default action。说明：widget 可通过 `xgeXuiWidgetSetDefaultAction` 设置默认动作；键盘事件无人消费时，FocusManager 在当前 focus scope 内按 TAB 顺序触发第一个可见启用 default action。
- [x] 支持 Escape cancel action。说明：widget 可通过 `xgeXuiWidgetSetCancelAction` 设置取消动作；键盘事件无人消费且没有 pointer capture 时，FocusManager 在当前 focus scope 内按 TAB 顺序触发第一个可见启用 cancel action。
- [x] 支持 Popup/Dialog focus restore。说明：Popup/Dialog 支持自动捕获当前焦点并在关闭后恢复；自动 restore 在关闭后清空，避免重复打开恢复到过期焦点；显式 `SetFocusRestore` 继续保留，用于 Menu/ComboBox owner 恢复。
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
- [x] Slider、Splitter、ScrollBar thumb、拖拽控件改用 pointer capture。说明：Button/IconButton、CheckBox、Radio、Switch、Toggle、StatusBar、Tabs、Toolbar、Input/TextEdit selection、Slider、Splitter、SplitLayout、Window、ScrollBar、ScrollView、ListView、VirtualList、TreeView、TableView、PropertyGrid 等内置 capture 使用点已按事件 `iPointerId` 设置、查询和释放 capture；内部 active/selection 状态仍为单 pointer 语义，需要多指手势的控件后续单独扩展。
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
  - [x] VirtualList 已迁移到 Widget 基础背景绘制，VirtualList paint 只保留滚动条绘制。
  - [x] ListView 已迁移到 Widget 基础背景绘制，ListView paint 只保留列表项、滚动条和现有边框绘制。
  - [x] TreeView 已迁移到 Widget 基础背景绘制，TreeView paint 只保留节点行、展开器和滚动条绘制。
  - [x] TableView 已迁移到 Widget 基础背景绘制，TableView paint 只保留表头、单元格、网格线和滚动条绘制。
  - [x] PropertyGrid 已迁移到 Widget 基础背景绘制，PropertyGrid paint 只保留属性行、编辑态和滚动条绘制。
  - [x] Tabs 已迁移到 Widget 基础背景绘制，Tabs paint 只保留焦点覆盖和 tab 项绘制。
  - [x] Toolbar 已迁移到 Widget 基础背景绘制，Toolbar paint 只保留工具项、分隔符和 overflow 绘制。
  - [x] StatusBar 已迁移到 Widget 基础背景绘制，StatusBar paint 只保留顶部边线、状态项和进度绘制。
  - [x] Breadcrumb 已迁移到 Widget 基础背景绘制，Breadcrumb paint 只保留 segment、separator 和文本绘制。
  - [x] Accordion 已迁移到 Widget 基础背景绘制，Accordion paint 只保留 header、content、边框和文本绘制。
  - [x] Window 已迁移到 Widget 基础背景绘制，Window paint 只保留外框、标题栏、按钮和 preview 绘制。
  - [x] ColorPicker 已迁移到 Widget 基础背景绘制，ColorPicker paint 只保留 swatch、field、palette、边框和文本绘制。
  - [x] TextEdit 已迁移到 Widget 基础背景绘制，TextEdit paint 只保留行号、selection、highlight、文本、光标、滚动条和边框绘制。
  - [x] Input 已迁移到 Widget 基础背景绘制。normal/disabled/error 背景状态由 Input 同步到 `pWidget->tStyle.iBackgroundColor`，Input paint 不再绘制本体背景。
  - [x] Toast 已迁移到 item surface 绘制。Toast overlay widget 本体不绘制 background/border，`backgroundColor`/`borderColor` 进入 `tItemStyle` 和每条 item 的 `tStyle`。
  - [x] Menu 已拆分 panel/background 与 frame/border 语义，菜单面板背景同步到 Popup Widget 基础背景，边框使用独立 `iBorderColor`。
  - [x] Dialog/MessageBox 已迁移到 Widget 基础背景/边框绘制。modal backdrop 通过 Widget `paintBefore`/underlay hook 绘制，先于面板背景且不受 Dialog 本体 content clip 截断。
- [x] 保持文本和图标编排在具体控件层。

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
- [x] 支持 selected/hover/focus index 与 slot 解耦。说明：VirtualScrollViewBase 维护 `iSelected`、`iHover`、`iFocus` 逻辑 item index，并提供对应 setter/getter；VirtualList、ListView、TreeView、TableView、PropertyGrid 已同步到 `tBase`，slot recycle、visible window 和控件私有稳定 id 不再影响这三类 index。
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
- [x] VirtualList 只允许 itemTemplate。
- [x] Overlay 使用 layer / portal / owner 进入 overlay root。说明：基础 OverlayManager API 已支持 layer、owner 和 overlay root 挂载；XSON `popup`、`dialog`、`messageBox` 声明节点通过 portal 进入 overlay root，并在 `PageSyncStyle`/`PageUnload` 中按 path 注册表刷新和释放；Menu/Tooltip 的实际浮层分别通过 MenuInit/context tooltip service 接入 overlay root，独立 `type:"tooltip"` 仍仅作为 owner 绑定兼容写法。
- [x] 支持字段：overflow、zIndex、layer、tabStop、tabIndex、imeMode、hitTestVisible、inputTransparent、borderColor、borderWidth、focusRingColor、focusRingWidth、disabledOverlay、debugOutlineColor、debugOutlineWidth。
- [x] 支持 ScrollViewBase 字段：wheelAxis、dragMode、scrollbarDrag、nestedScroll。
- [x] 错误信息带字段路径和 role 规则。
- [x] 更新 XSON 示例和 guide。说明：guide、基础 XSON 字段回归和聚合控件示例已同步；`xui_xson_controls_lab` 覆盖基础/选择/范围/ColorPicker/DatePicker/数据/体验控件和 tooltip。

## 阶段 L：控件迁移与成熟度重验

- [x] Button、IconButton、Label、Image 重验裁剪、状态、事件、焦点。说明：四者均固定为 Control role 默认 `overflow: clip`；Button/IconButton 覆盖 focus in/out、鼠标/键盘事件、默认裁剪和 `overflow: visible` 放开裁剪；Label/Image 覆盖默认 Control 裁剪、不默认焦点和文本/图片裁剪路径。
- [x] Input、TextEdit 重验 IME、selection、文字裁剪、焦点。说明：两者均固定 Control role 默认 `overflow: clip`、focusable 和 IME enabled；focused readonly 会主动切到 IME disabled，readonly 解除后恢复 IME；Input 覆盖 password/readonly/disabled IME、selection 替换、鼠标拖选、双击选词、IME composition/candidate；TextEdit 覆盖 readonly IME、selection、键盘/鼠标选择、undo/redo、滚动、word wrap、candidate 和文本裁剪路径。
- [x] CheckBox、Radio、Switch、Slider、Progress 重验事件和 pointer capture。说明：CheckBox、Radio、Switch 固定 Control role、默认 `overflow: clip`、IME disabled、focusable、鼠标/键盘切换、外部松开不触发变更、Escape capture lost/cancel 清 active；Slider 固定 Control role、focus/capture、拖拽、外部释放、Escape capture lost/cancel，并补齐键盘 Home/End/方向键/Page 调值；Progress 固定为只读 Control，不 focus、不注册事件、不抢 capture。
- [x] ScrollBar、ScrollView 重验 ScrollViewBase。说明：ScrollView 与 ScrollViewBase 已重验；独立 ScrollBar 固定 Control role、默认 `overflow: clip`、IME disabled、focusable，补齐按钮/轨道/thumb 按压 capture、Escape capture lost/cancel、横纵向拖拽、滚轮、键盘 Home/End/方向键/Page 调值和 disabled 回归。
- [x] ListView、VirtualList 重验 VirtualScrollViewBase。说明：VirtualList 直接等同于 VirtualScrollViewBase；ListView 已通过 `tBase` 重验通用滚动、选择、捕捉和 viewport 口径。
- [x] Popup、Menu、ComboBox、Tooltip、Dialog 重验 overlay layer/top 和 focus restore。说明：Popup/Dialog 固定 Overlay role、layer 与 IME disabled，补齐自动/显式 focus restore、Escape/outside close 和重复打开不过期恢复；Menu/ComboBox 通过 Popup overlay 恢复 owner/控件焦点并覆盖 overlay top；Tooltip 已作为 context 内部 overlay popup 覆盖 tooltip layer/top；完整 OverlayManager 统一接管仍由阶段 D 追踪。
- [x] TreeView、TableView、PropertyGrid 重验 Widget 适配。说明：TreeView、TableView、PropertyGrid 已通过 `tBase` 重验通用滚动、选择镜像、捕捉和 viewport 口径。
- [x] ColorPicker 补齐完整基础控件能力并重验。说明：ColorPicker 已支持 palette 点击选择、RGBA 数字字段键盘编辑、hex 字段键盘编辑、Enter 提交、Escape 取消、无效输入错误态、focus out 提交/取消和编辑态 IME 申请策略。
- [x] DatePicker 实现并纳入基础控件验收。说明：DatePicker 已支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 `examples/xui_date_picker_lab` 验证。
- [x] Toggle、NumericInput、SearchBox、Separator、Splitter 重验 Widget 适配。说明：这些单 widget 控件已覆盖 role、默认裁剪、事件/焦点/capture、IME 策略、状态和 Unit 清理。
- [x] Tabs、Toolbar、StatusBar、Breadcrumb、Accordion、Toast 重验复合控件口径。说明：已覆盖 role、布局/绘制、选择/点击回调、tooltip/overflow/队列等控件私有状态，并通过聚合 XSON 示例覆盖声明式入口。
- [x] Panel、Window、SplitLayout、MessageBox 重验容器/窗口/组合布局口径。说明：Panel/Window/MessageBox 已覆盖基础绘制、overlay/focus/关闭策略；SplitLayout 已覆盖 Container role、pane/divider 布局、divider capture、shadow drag 和 Unit 清理。
- [x] RichTextView、CodeEditor、NodeGraph、Timeline 建立可选高级组件路线，不阻塞核心。说明：这些控件按可选高级组件推进，不进入 Widget 基础层和基础应用控件成熟度阻塞项；核心侧仅保留 ScrollViewBase、VirtualScrollViewBase、文本输入、选择、焦点、clip 和绘制基础设施要求。

## 阻塞与决策记录

- [x] 决策：统一术语使用 Widget，不再引入 Element。
- [x] 决策：保留一个底层 Widget 类型，通过 role 区分 Control、Container、Viewport、Overlay。
- [x] 决策：ScrollViewBase 和 VirtualScrollViewBase 分成两层，不做单个大基类。
- [x] 决策：ScrollViewBase 内容拖拽默认关闭，避免干扰地图/画布/编辑器控件。
- [x] 决策：Widget 基类只绘制背景、边框、focus ring，不封装文本和图标编排。
- [x] 决策：IME 默认禁用，文本控件显式申请。
- [x] 决策：代码里继续使用 `widget`、`tRect` 等简洁兼容命名；`tRect` / `xgeXuiWidgetGetRect` 作为 borderRect 兼容入口，不在代码命名里加入版本号。
