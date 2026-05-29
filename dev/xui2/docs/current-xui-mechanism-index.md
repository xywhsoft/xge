# Current XUI Mechanism Index

本文是 XUI 重构前对当前 XUI 已实现机制的索引。目的不是要求 XUI 复制现有内部实现，而是避免重构时只保留显眼控件，丢失已经沉淀出的行为约定、调试能力、声明式能力和 APP/GAME 接入经验。

## 1. Scope

本索引覆盖以下来源：

| Source | Role |
| --- | --- |
| `xge.h` | XUI 公开 API、常量、结构体、事件、控件能力边界 |
| `src/xge_xui_core.c` | context、widget tree、event、focus、capture、tooltip、overlay、update、paint |
| `src/xge_xui_host.c` | host bridge、draw/clip/text/texture/font/clipboard/time backend |
| `src/xge_xui_layout.c` | style default、theme style、measure/layout、size resolve |
| `src/xge_xui_page.c` | XSON page、imports、tokens、style inheritance、templates、binder、model、overlay portal |
| `src/xge_xui_*.c` | 标准控件、复合控件、数据控件、overlay 控件 |
| `docs/xui` / `docs/guide` / `dev/docs` | 现有设计说明、SPEC、入门文档 |
| `examples/xui_*` | 控件和机制人工验证样例 |
| `test/xui_*` | null host、DockPanel、MsgTip、debug snapshot smoke tests |

XUI 重构时，每个机制都应被标记为：

- `keep`: 语义和能力保留，可换实现。
- `replace`: 保留目标能力，但重做架构。
- `standard-control`: 不进核心，作为标准控件迁移。
- `legacy`: 只兼容旧页面或旧工具链，不作为 XUI 推荐路径。
- `drop`: 明确废弃，并记录替代方案。

## 2. Public API Surface

当前 XUI 的公开 API 大致分为以下家族：

| Family | Existing Coverage |
| --- | --- |
| Core/context | init/unit、root、overlay root、DIP、safe area、theme、tokens、refresh、layout batch、host |
| Resource | XUI font register/create/destroy、texture create/get/destroy、builtin atlas asset |
| Page/XSON | load/unload/find、style refresh/sync、model apply、binder click/event |
| Widget | tree、id/name/role、rect、style、layout、state style、visibility、focus、IME、hit test、event、measure/layout/paint、tooltip、dirty |
| Event/input | dispatch、queued events、hotkey、command、focus、capture、pointer capture、drag、IME |
| Text core | UTF-8 buffer、insert/delete、cursor、selection、composition、text input event |
| Controls | button、label、image、input、textEdit、choice、checkbox、radio、toggle、numericInput、colorPicker、datePicker、comboBox |
| Containers/data | panel、tabs、scroll model/frame/view、listView、treeView、tableView、tableGrid、propertyGrid、accordion、timelineView |
| Overlay/window | popup、menu、menuBar、window、msgBox、inputBox、msgTip、toast、toolbar/statusBar |
| Layout controls | splitLayout、dockLayout、dockWindow、dockPane |
| Debug | widget tree dump、layout snapshot、inspect、debug overlay、page trace、event trace |

XUI 可以重排模块边界，但公开能力不应只按源码文件迁移。很多行为由 core、page 和控件共同完成，例如 ComboBox 同时依赖 button 状态、popup placement、scroll/list 能力和 focus restore。

## 3. Core Context

当前 context 已经承担以下职责：

- 创建并维护 `root` 与 `overlayRoot` 两棵根。
- 保存当前 theme、chrome style、registered tokens、theme version。
- 保存 host V1/V2，隔离 XGE draw、clip、text、texture、clipboard、time。
- 管理 DIP scale、safe area、refresh request、layout batch。
- 管理当前 focus、hover、capture、pointer capture、drag target、tooltip owner。
- 维护 hotkey 表、event queue、dirty layout/paint 统计。
- 支持 `xgeXuiDispatchProcFrameEventAll()` 这种 ProcFrame 集成方式。

XUI 应保留的核心点：

- `Context` 仍是 UI world 的根，不能让控件直接依赖具体窗口或渲染后端。
- `Root` 和 `OverlayRoot` 应继续分离，overlay 不应污染普通布局树。
- `Host/Backend` 必须继续可替换，null host 对自动测试很重要。
- `RefreshRequest/RefreshNeeded/RefreshClear` 的 APP 接入语义应升级为完整 damage/present 机制，而不是删除。
- Theme/token/version 的概念应进入新的 StyleManager，而不是散落在控件初始化里。

## 4. Widget Base

当前 widget 是 XUI 的最大资产。它已经不是简单矩形节点，而是包含大量基础 UI 语义。

### 4.1 Tree and Identity

已支持：

- parent/children/sibling tree。
- `id`、`name`、tree order。
- `role`: control/container/viewport/overlay。
- 普通 add/remove 和 internal add。
- find by id/name。
- remove/free 时清理 focus、capture、hotkey、tooltip 等上下文引用。

XUI 注意点：

- Tree 操作必须继续触发 layout、paint、event mask、hit-test order 更新。
- 复合控件内部 child 需要保留“内部节点”语义，避免用户误删或 XSON 错挂 children。
- Tree order 是 layer/z 相同后的稳定排序条件，重构时不能丢。

### 4.2 Box Model

已支持：

- local rect、outer rect、border rect、padding rect、content rect。
- margin、padding、border、radius、background、focus ring、disabled overlay、debug outline。
- width/height/min/max，size unit 包括 `px`、`dip`、`percent`、`content`、`grow`。
- anchor left/top/right/bottom。
- alignX/alignY、justify、gap。

XUI 注意点：

- 缓存矩形应明确选择 border box 还是 content box。当前背景、边框、focus ring 主要以 border rect 绘制，children layout 主要使用 content rect。
- `content` size 依赖 measure，XUI 的 dirty layout 应能从内容变化向上冒泡。
- `dip` 与后端 scale、字体度量有关，应纳入 layout cache key。

### 4.3 Flags and State

已支持：

- visible、enabled、focusable、tab stop、focus scope。
- clip、hitTestVisible、inputTransparent、dragEnabled。
- dirty layout、dirty paint、dirty style。
- visual state: normal/hover/active/focus/disabled/checked。
- state style: normal/hover/active/focus/disabled/checked 的绘制字段覆盖。

XUI 注意点：

- `visible/enabled` 不只是绘制开关，还会影响 focus、capture、event、layout。
- `hitTestVisible/inputTransparent` 是 overlay、装饰层和穿透交互的基础。
- state style 目前只覆盖有限绘制字段；XUI 多状态缓存需要把 state variant 作为一等概念。

## 5. Layout

当前核心 layout 已支持：

| Layout | Existing Use |
| --- | --- |
| `absolute` | 固定坐标、anchor、window 内部、split/dock 控件内部 |
| `row` | 横向工具条、表单行、状态栏元素 |
| `column` | 面板、设置页、accordion section |
| `stack` | 重叠层、背景/前景组合 |
| `grid` | 固定列数、固定行高、简单图标/卡片矩阵 |
| `dock` | top/bottom/left/right/fill APP shell |

已支持的布局输入：

- child width/height/min/max。
- margin、padding、gap。
- align/justify。
- grid columns、row height、column gap、row gap、column span。
- dock side。
- custom measure proc、custom layout proc。
- layout batch begin/end。

XUI 应保留并改进：

- 保留 `absolute/stack/row/column/dock/grid` 的心智和行为。
- 新增 `table` 和 `flow` 时，不破坏 APP 布局确定性。
- 把 measure/arrange 约束模型正式化：available size、desired size、arranged rect、content size。
- Layout dirty 应能区分 self size、child size、position only，避免全树重排。
- 复杂控件如 splitLayout、DockPanel、Viewport、VirtualList 不进入核心 layout enum。

## 6. Rendering and Paint

当前渲染链路：

```text
xgeXuiPaint()
  -> layout if dirty
  -> root paint
  -> overlay root paint
  -> widget paint before/content/children/after
  -> host draw rect/rounded/border/text/image/shape
```

已支持：

- Paint clip stack，兼容 host 原始 clip。
- 基于 widget `clip`/`overflow` 的 content rect 裁剪。
- `procPaintBefore`、`procPaint`、`procPaintAfter`。
- Widget owner draw: none/content/content_and_children/full。
- PaintInfo 包含 widget、context、rect、state、style、control pointer。
- Host V2 提供 draw/clip/text measure/texture/font/clipboard/time 能力。
- paint 时按 layer、z、tree order 排序。
- 外部 surface dirty generation 触发 repaint。

当前限制：

- dirty rect 更像刷新提示，不是真正 retained damage/composite。
- 即使只有局部变化，也可能进入整棵 tree paint。
- 控件 paint 与系统 paint 耦合，静态控件会重复 draw 背景、边框、文字。
- 裁剪是在 draw 过程中动态维护，不是缓存合成阶段的统一裁剪。

XUI 对应方向：

- 将 widget draw 与 compositor draw 分离。
- 控件更新自身 local/state cache，系统只合成缓存。
- 保留 owner draw 能力，但要明确它使缓存失效或成为 custom raster callback。
- Host V2 应升级成 backend/surface abstraction，支持 bitmap cache、texture cache、partial present fallback。

## 7. Event and Input

当前事件系统已经比较完整，不能在 XUI 中弱化。

已支持：

- Raw XGE event 到 XUI semantic event 的转换。
- event phase: tunnel、target、bubble。
- event result: continue、handled、consumed。
- widget event proc、capture event proc、per-event handler。
- event interest mask 和 subtree event mask，减少 hot path 命中。
- hit test 按 overlay/root、layer、z、tree order、clip、hitTestVisible、inputTransparent 判断。
- mouse enter/leave/move/down/up/wheel。
- click、double click、context menu。
- pointer capture，多 pointer id capture。
- drag begin/move/end/cancel。
- focus in/out、TAB focus、focus scope。
- default action、cancel action。
- hotkey registration、command dispatch。
- event queue push/pop/dispatch。
- long press/context timing。

XUI 注意点：

- Event mask 是性能机制，重构时应继续保留，并可扩展成 subtree capability flags。
- Pointer capture 和 drag 状态要与 widget 生命周期强绑定，节点删除、隐藏、disable 时必须自动清理。
- Overlay/modal/popup 对 hit test、focus、escape、outside close 有特殊影响，不能只做绘制层。
- APPUI 模式下，事件触发的状态变化应产生精确 style/layout/cache damage。

## 8. Focus, Keyboard and IME

已支持：

- focusable/tabStop/tabIndex/focusScope。
- `xgeXuiSetFocus()` 发送 focus in/out 并标记 repaint。
- TAB/Shift+TAB focus stepping。
- Enter/default action、Escape/cancel action。
- IME mode: disabled/enabled/auto。
- IME candidate rect callback 与 context candidate rect 查询。
- Text core 支持 composition start/update/end 相关输入路径。

XUI 注意点：

- Focus 变化不仅影响事件目标，还影响 state cache variant。
- IME candidate rect 必须基于最终屏幕位置，滚动/缓存/transform 后也要正确。
- TextEdit/Input 的输入策略样例很多，重构时要保留导航、删除、替换、剪贴板、密码、选择等 policy 能力。

## 9. Overlay, Popup and Window

已支持：

- Overlay root 独立于普通 root。
- overlay attach/detach/bringToFront/top/owner。
- layers: normal、floating、popup、modal、tooltip、drag adorner、debug。
- Popup: owner、open、modal、auto close、placement、anchor rect、offset、focus restore、content widget、content size、match owner width、close/focus policy、inside consume、scroll。
- Menu/MenuBar: popup menu、items、separator、state、select、metrics/colors。
- ComboBox: button + popup + list behavior。
- Window: floating layer、active window、title bar、move、resize、collapse、maximize、topmost、client widget。
- MsgBox/InputBox/MsgTip/Toast: modal/notification overlay patterns。
- Tooltip: desc/resolver/open owner/rect，内部基于 overlay。

XUI 注意点：

- Overlay 是架构层，不只是控件。缓存、damage、hit test、focus、modal input 都需要 overlay-aware。
- Popup placement 已有较多策略，应迁移为独立 placement 服务，供 popup/menu/comboBox/tooltip/datePicker/colorPicker 复用。
- Window/DockPanel/SplitLayout 属于标准控件级复杂能力，不应塞进 widget core。

## 10. XSON Page System

当前 XSON 机制已经相当丰富。

已支持：

- Page load from URI/memory，resource copy 和 parse error。
- `imports`，含 URI normalize、relative resolve、cycle detection、depth/capacity 限制。
- imports 合并 `styles`、`tokens`、`templates`。
- `tree` 构建 retained widget tree。
- type 到 layout/role/control initializer 的映射。
- overlay portal：popup/menu 等节点可挂到 overlay root，同时保留 XSON path 索引。
- id/name 索引，支持 numeric id 和 string id。
- named `styles`。
- `@parent` style inheritance，含 parent chain depth 和 cycle detection。
- page/context tokens，支持 colors/spacing/fonts/textures 命名空间和 fallback。
- widget inline 字段覆盖 style 字段。
- safeArea 文档字段映射到 root padding。
- widget common fields: layout、size、margin、padding、border、background、state、tooltip、focus、IME、events 等。
- control private fields 按控件解析。
- binder click/event。
- hotkey binding。
- model binding，支持 text/int/float 写入 model 后 `xgeXuiPageApplyModel()` 更新 label/input/image 等。
- style version，`xgeXuiPageRefreshStyle()` / `xgeXuiPageSyncStyle()`。
- page error string 与 debug trace。

当前限制：

- XSON style inheritance 发生在 Page 层，Widget C API 层没有父子样式继承。
- 没有 class/selector/cascade/pseudo/media。
- token/theme 变化需要显式 sync/refresh。
- 控件私有字段和通用 widget style 解析入口不统一。
- 模板、事件、model 是轻量能力，不是完整脚本或数据绑定框架。

XUI 对应方向：

- 继续使用 XSON/JSON 作为 page/theme/style 文件格式。
- 将 imports/tokens/named style/template/error path 等成熟机制迁移到 StyleManager/PageBuilder。
- 把 style refresh/sync 升级成 diff + dirty category broadcast。
- 保留无脚本原则，交互通过 binder、command、model adapter。

## 11. Current Style and Theme

当前样式分为多层，但还没有统一 StyleManager。

已支持：

- `xgeXuiThemeDefault()` 和 context theme/themeVersion。
- context registered tokens: color/spacing/font/texture。
- `xgeXuiStyleDefault()`、`xgeXuiStyleFromTheme()`。
- widget 持有一份 `xge_xui_style_t`。
- `xgeXuiWidgetSetStyle()` 直接整体拷贝 style。
- state style 覆盖 background、border、focus ring、disabled overlay 等绘制字段。
- chrome style：window、toolbar、menubar、statusbar 等 APP chrome metrics/colors。
- 控件初始化从 theme/chrome style 读取默认颜色、字体、间距。
- XSON named style + `@parent` + tokens + inline override。
- TableView column/row/cell 局部 style。
- PropertyGrid 自有 style 结构。
- Button/Progress 支持 nine-patch state/track/fill。

XUI 注意点：

- Theme、chrome style、named style、state style、控件私有 style 应统一纳入 typed style system。
- Widget 热路径只读 resolved style/cache id，不查 XSON、不解析 token。
- Style diff 必须输出 dirty category：layout、paint、cache、text measure、resource。
- 动态换肤要能广播，但不能强制全树重建。

## 12. Scroll and Viewport

当前滚动系统由三层组成：

| Layer | Responsibility |
| --- | --- |
| ScrollModel | viewport/content size、offset clamp、ensure rect visible、坐标转换 |
| ScrollFrame | viewport widget、H/V scrollbar、corner、policy、mode、wheel/content drag、layout/event/colors |
| ScrollView | ScrollModel + ScrollFrame + content widget，给普通内容提供滚动容器 |

复用 ScrollFrame/ScrollModel 的控件：

- ListView。
- TreeView。
- TableView。
- PropertyGrid/TableGrid。
- Popup 内部滚动。

已支持：

- scrollbar policy: auto/always/hidden。
- scrollbar mode: full/compact。
- wheel axis/step。
- content drag。
- content size 与 viewport size 同步。
- ensure child/rect visible。
- viewport/content/screen 坐标转换。

XUI 注意点：

- `flow` 本身不应内嵌 scroll frame；DocumentView/FlowView/ScrollView 负责 viewport。
- Dirty/cache 与 scroll offset 要配合：静态 content cache 可以随 offset 合成，viewport damage 只需暴露区域重绘。
- 数据控件需要保留 viewport paint proc 和可见项计算，不应退化成全量 child widget。

## 13. Text, Input and Editing

已支持：

- `xge_xui_text_t` UTF-8 文本缓冲。
- set/insert/insert codepoint/delete back/delete forward。
- cursor、selection、composition。
- Text input event 处理。
- Input 控件：文本、placeholder、password、selection、cursor、IME、clipboard/nav/delete/replace/context policy。
- TextEdit 控件：多行、wrap/scroll、undo stack/history limit、selection、context menu/detail 等样例覆盖。
- NumericInput：数值范围、步进、格式、编辑。
- InputBox：modal input overlay。

XUI 注意点：

- Text 不是普通 label 的附属实现，应作为共享文本编辑核心。
- 文本 measure/layout/cache 对 APPUI 性能影响很大，XUI 应缓存 glyph/layout run。
- IME、selection、cursor blink 是持续动态状态，应避免让整个父 cache 频繁失效。

## 14. Resource and Asset Mechanisms

已支持：

- XUI font create from file/memory/register/get/destroy/default font。
- XUI texture create from RGBA/memory/file/get desc/destroy。
- Host V2 texture/font backend。
- builtin atlas PNG 与 named asset rect。
- 内置图标用于 checkbox/radio/menu/button badge/toast/combo arrow 等。
- NinePatch 基础绘制，Button/Progress 已直接使用。

XUI 注意点：

- Cache surface、texture、font atlas、builtin atlas 应归入统一 resource manager。
- Bitmap cache 需要明确 ARGB premultiplied、stride、DPI、color space、lifetime。
- 控件 state cache 与 nine-patch/resource version 绑定，资源变更必须使缓存失效。

## 15. Standard Controls Inventory

以下控件能力需要迁移索引，而不只是迁移文件名。

### 15.1 Basic Visual Controls

| Control | Key Mechanisms |
| --- | --- |
| Label | text/font/color/align/measure/paint/XSON/model text |
| Image | texture/src/fit/tint/measure/XSON/model texture |
| Separator | orientation/line style/color/metrics |
| Progress | value/range/direction/text/track/fill/nine-patch |
| Slider | value/range/step/orientation/focus/capture |
| ScrollBar | orientation/range/page/thumb/buttons/capture |
| Panel | container defaults/background/border/layout host |

### 15.2 Action and Choice Controls

| Control | Key Mechanisms |
| --- | --- |
| Button | click/selectable/selected/semantic/icon/icon layout/badge/state colors/nine-patch |
| CheckBox | checked/change/text/icon/state/keyboard |
| Radio/RadioGroup | checked/group exclusive/change/text |
| Toggle | checked/text/on-off visual/state |
| Tabs | tab buttons/pages/selection/orientation/style sync |
| Pager | page index/page count/buttons/range display |

### 15.3 Input Controls

| Control | Key Mechanisms |
| --- | --- |
| Input | single-line editing, placeholder, password, IME, selection, clipboard policies |
| TextEdit | multi-line editing, wrap, scroll, undo/history, selection |
| NumericInput | value clamp, step, text conversion, spinner/edit |
| ColorPicker | popup/palette/value/change |
| DatePicker | popup/calendar/date value/change |
| ComboBox | popup/list/items/enabled items/selected value |
| InputBox | modal prompt/result/focus restore |

### 15.4 Data and Compound Controls

| Control | Key Mechanisms |
| --- | --- |
| ListView | items, selection, hover, scroll frame, item renderer |
| TreeView | nodes, expanded/collapsed, visible list, selection, indent, scroll |
| TableView | columns/rows/cells, selection, hover, scroll, cell renderer, column/row/cell style |
| TableGrid | editable grid, editor config, active editor, commit/cancel |
| PropertyGrid | categories/properties, typed values, validation/change/action, editor integration |
| TimelineView | layers/frames/spans/ruler, hit test, selection, renderers |
| Accordion | sections, header button, expand/collapse, client widgets |

### 15.5 Overlay and APP Chrome Controls

| Control | Key Mechanisms |
| --- | --- |
| Popup | placement, modal, close/focus policy, content widget, scroll |
| Menu/MenuBar | items, separators, checked/disabled, nested popup behavior |
| Window | title bar, move/resize/collapse/maximize/topmost/client widget |
| MsgBox | modal backdrop, buttons, result |
| MsgTip | transient message, duration, type, icon |
| Toast | notification queue, placement, direction, close/click callbacks |
| Toolbar | items/groups/overflow/chrome metrics |
| StatusBar | segments/progress/chrome metrics |

### 15.6 Layout/Workbench Controls

| Control | Key Mechanisms |
| --- | --- |
| SplitLayout | panes, splitter drag, ratios, min sizes, preview |
| DockLayout/DockPanel | dock windows, panes, tabs, floating/docking behavior |
| DockWindow/DockPane | IDE-like workbench primitives |
| ScrollView | viewport + content widget scroll container |

XUI 注意点：

- 复合控件应优先复用 widget、layout、style、scroll、popup、command 基础设施。
- 数据控件不宜把每个 item 都强制变成 widget；虚拟化和 item/cell renderer 是性能关键。
- 每个控件 setter 应声明会触发 layout、paint、cache、data 或 style 哪类失效。

## 16. Debug and Diagnostics

已支持：

- `xgedbgXuiWidgetTreeDump()`。
- `xgedbgXuiLayoutSnapshot()`。
- `xgedbgXuiWidgetInspect()` / `InspectAt()`。
- `xgedbgXuiDebugOverlayPaint()`。
- `xgedbgXuiPageTrace()`。
- `xgedbgXuiEventTrace()`。
- Paint host lab、dirty rect lab、focus/capture/event lab、layout validation examples。

XUI 必须保留甚至加强：

- Widget tree inspect。
- Layout snapshot。
- Render/cache/damage graph inspect。
- Event route trace。
- Style resolve trace。
- Dirty reason trace。
- Null backend smoke tests。

重构后如果没有这些工具，缓存、dirty、style diff 和 layout negotiation 会很难调试。

## 17. Existing Example Coverage

当前 `examples/xui_*` 覆盖面很广，应作为迁移验收清单：

- 基础控件：button、label、image、separator、progress、slider、panel。
- 输入：input、text buffer/delete/select/undo/wrap/IME/clipboard/password/context/nav/policy、textEdit、numericInput。
- 选择：checkbox/radio/choice/toggle/tabs/pager/comboBox。
- 数据：listView、treeView、tableView、tableGrid、propertyGrid、timelineView、accordion。
- 容器和布局：layout gallery、layout validation、scrollView、scrollbar、splitLayout、dockPanel。
- overlay：popup、popup owner/policy、menu、menuBar、window、msgBox/inputBox、msgTip、toast、tooltip。
- XSON：大多数控件均有 `_xson` 示例，另有 app layout、theme、widget tree style lab。
- 调试/宿主：paint host lab、visual proc lab、focus capture event lab、tabs dirty rect lab。

XUI 第一阶段不一定迁移全部控件，但应在 roadmap 中记录每个示例对应的新归属。

## 18. Known Gaps to Fix in XUI

当前机制中最值得重构解决的问题：

- 没有完整缓存/合成/damage renderer，APPUI 脏矩形无法真正省掉控件 draw。
- dirty layout/paint/style 是粗粒度 flag，缺少 dirty reason 和失效范围分类。
- 样式没有运行时 StyleManager、class、selector、继承属性和 diff broadcast。
- 控件私有样式分散在各控件结构中，无法统一换肤。
- Layout 有 APP 基础能力，但缺少 flow/document 和 table track negotiation。
- 滚动、popup、数据控件中有不少局部策略，尚未抽象成共享服务。
- 文本 layout/glyph 缓存不足，APP 文档/表单场景会重复 measure/draw。
- Page/XSON 能力较强，但与 C API widget runtime 的样式模型割裂。

## 19. XUI Migration Checklist

重构推进时，每个模块至少完成以下记录：

1. 当前 XUI 机制归属：core、page、style、layout、render、event、standard control、legacy。
2. XUI 目标归属：是否进入核心，还是标准控件/工具链。
3. 行为兼容：公开 API 是否兼容，XSON 字段是否兼容，语义是否改变。
4. Dirty impact：状态变化触发 layout、paint、cache、damage、style 哪些失效。
5. Cache policy：local cache、state cache、subtree cache、display-list cache、no-cache。
6. Test/example：用哪个现有 example 或 smoke test 验收。
7. Debug support：tree/layout/event/style/cache trace 是否可观察。

最低迁移顺序建议：

1. Context、host/backend、resource、root/overlay root。
2. Widget tree、box model、event/focus/capture、layout base。
3. Render node、cache surface、damage/compositor。
4. StyleManager、tokens、resolved style、style diff。
5. Page/XSON builder、imports/templates/binder/model。
6. 基础控件与输入控件。
7. Scroll/viewport、popup/menu/comboBox。
8. 数据控件、DockPanel/SplitLayout/Window 等复杂控件。

## 20. Preservation Principle

XUI 可以大胆重构，但不能只重构“画控件”这件事。当前 XUI 已经积累了四类容易被低估的资产：

- UI runtime 资产：tree、layout、event、focus、capture、overlay、IME。
- APP 资产：host、refresh、safe area、chrome、popup、window、toolbar/statusbar。
- Tooling 资产：XSON、imports、tokens、style inheritance、binder、model、debug trace。
- Control 资产：数据控件、编辑器、scroll viewport、item/cell renderer、复杂 overlay 控件。

XUI 的正确目标不是复制旧代码，而是把这些资产重新放到更清晰的架构位置上，并让缓存、APPUI、布局和样式系统成为从第一天就存在的基础设施。

