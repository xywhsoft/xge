# XUI API

> XUI API 是 XGE 内部孵化期 GUI 接口，负责布局、控件、事件响应和通过 XGE host 渲染。

[返回 API 索引](README.md) | [XUI 教程](../guide/xui-bridge-intro.md) | [XUI 范例](../case/xui-bridge.md)

---

## 模块定位

XUI 的长期目标是成为可独立工作的 DUI 系统。当前阶段先在 XGE 内孵化，方便调试和回归测试；成熟后再剥离到 XUI 仓库，并通过高性能 bridge 接入 XGE。

边界：

- XGE 提供时间、输入、渲染能力和资源对象。
- XUI 负责布局、控件树、事件响应、焦点/捕获、dirty 标记和 paint。
- XUI 不内置完整 IME，第一版依赖系统 IME，XUI 处理提交文本和候选区域。

## 标准调用顺序

```text
xgeXuiInit
  -> xgeXuiSetHost 可选
  -> 创建 widget 树和控件
  -> xgeXuiDispatchEvent / xgeXuiEventPush
  -> xgeXuiUpdate
  -> xgeXuiPaint
  -> xgeXuiUnit
```

## 控件标准契约

### 状态

基础交互状态为 `normal`、`hover`、`active`、`focus`、`disabled`。公共状态位优先保留给基础交互；`checked`、`selected`、`open`、`readonly`、`error`、`loading`、`dirty` 第一版优先保留在控件私有字段或控件专用 item 状态中，只有跨控件热路径需要统一判断时再提升为公共 `XGE_XUI_STATE_*` bit。

状态优先级为：`disabled > active > hover > focus > checked/selected > normal`。语义状态 `error`、`loading`、`dirty` 作为叠加状态，不覆盖基础交互状态；例如 error input 仍可显示 focus，loading button 仍消费激活事件但不触发 click。

### 事件命名

标准事件命名采用 PascalCase：`Click`、`Change`、`Changing`、`Select`、`Submit`、`Cancel`、`Open`、`Close`。

- `Click`：离散命令被确认触发，例如 Button、Toolbar item、StatusBar clickable item。
- `Changing`：拖拽或连续编辑中的中间值变化；第一版已预留命名，控件可按需后续接入。
- `Change`：值已提交到控件状态后的变化，例如 Slider 释放或键盘步进、Input 文本实际改变、Toggle checked 改变。
- `Select`：选择目标改变后触发，disabled item 不触发；多选控件以当前焦点项或主选择项作为回调 index。
- `Submit`：输入类控件确认当前文本或值，通常由 Enter、搜索提交或失焦提交触发。
- `Cancel`：取消编辑、关闭浮层或恢复前值，通常由 Escape 或外部取消路径触发。
- `Open` / `Close`：浮层、下拉、折叠区、toast/window 生命周期变化。

### 键盘与焦点

所有 focusable 控件应至少支持焦点进入、绘制焦点状态、禁用后不再接受新焦点。常用键位约定：

- `Enter`：触发默认确认、提交或当前焦点项激活。
- `Space`：触发按钮类、选择类控件的离散切换。
- `Escape`：取消当前编辑或关闭最上层可关闭 overlay；无可处理目标时继续传播。
- 方向键：在列表、树、表格、tabs、菜单等选择类控件中移动焦点或选择。

### Readonly 与 Disabled

`readonly` 和 `disabled` 不是同一状态。`readonly` 表示可聚焦、可选择、可复制，但不能修改值；`disabled` 表示控件不可交互，不应接受新焦点，通常使用 disabled 视觉。

输入控件的标准策略是：readonly 阻止文本输入、粘贴、剪切、删除、clear、IME 提交和会修改值的快捷键；复制、选择、光标移动保留。disabled 由 widget enabled 状态和控件私有 disabled 字段共同决定，事件通常直接继续传播或被父级策略跳过。

### Dirty 标记

setter 必须标记与结果匹配的 dirty：

- 影响布局、尺寸、padding、item count、字体、wrap、行号栏宽度的 setter 标记 layout 和 paint。
- 只影响颜色、文本内容、选择、hover、active、scroll、highlight 的 setter 标记 paint。
- 批量修改时使用 `xgeXuiLayoutBatchBegin` / `xgeXuiLayoutBatchEnd` 合并 dirty 传播。
- 控件内部事件导致状态变化时，应同步标记 dirty 并按需请求刷新。

### 生命周期与所有权

控件对象和 widget 结构体默认由调用者持有。`Init` 负责初始化控件状态、安装 widget 回调、设置必要 flags，并记录控件对 widget/context/font/texture 的引用；`Unit` 负责释放控件内部持有的缓冲、菜单、临时数组，并在需要时清理 widget 回调和 `pUser`。

除非 API 明确说明，传入的 font、texture、items、highlight ranges、adapter、字符串常量和用户指针都由调用方持有，控件只保存引用。所有 `Unit` 和大多数 setter 必须保持 `NULL` 安全；无效参数应直接返回或返回错误码，不破坏已有状态。

## 公共类型

### `xge_xui_context_t`

XUI 上下文，保存 root、focus、pointer capture、事件队列、主题、dirty 状态和 host。调用者可以透明读取状态，但不应直接破坏链表、队列或 capture 表字段。

### `xge_xui_widget_t`

基础 widget 节点，包含父子兄弟链表、ID、Name、style、rect、content rect、flags、事件回调、measure 回调和 paint 回调。

### `xge_event_t` 的 XUI 路由字段

`xgeXuiDispatchEvent` 传给 widget 回调的是事件副本，副本会补齐 XUI 路由信息：

- `iXuiPhase`：当前阶段，取 `XGE_XUI_EVENT_PHASE_TUNNEL`、`XGE_XUI_EVENT_PHASE_TARGET` 或 `XGE_XUI_EVENT_PHASE_BUBBLE`。
- `iPointerId`：坐标事件的 pointer id；鼠标默认为 `0`，触摸事件使用当前触点 id。
- `pXuiOriginalTarget`：原始命中目标；有 pointer capture 时仍保留命中目标，未命中时回落到 capture widget。
- `pXuiCurrentTarget`：当前正在回调的 widget。
- `pXuiCapture` / `bXuiCaptured`：当前事件是否因对应 `iPointerId` 的 pointer capture 改派到捕获 widget。

XUI 事件回调返回值分为三档：

- `XGE_XUI_EVENT_CONTINUE`：当前回调未处理事件，继续路由；如果全路由都返回 continue，最终返回给 XGE 处理。
- `XGE_XUI_EVENT_HANDLED`：当前回调已处理事件，但允许当前路由继续向后执行；最终不会交给 XGE，也不会触发 default/cancel fallback。
- `XGE_XUI_EVENT_CONSUMED`：当前回调已消费事件，立即停止后续 tunnel、target 或 bubble 路由。

### `xge_xui_host_t`

XUI 到宿主的绘制桥。

| 回调 | 说明 |
| --- | --- |
| `draw_rect` | 绘制矩形。 |
| `draw_image` | 绘制图片。 |
| `draw_text_rect` | 绘制文本。 |
| `measure_text` | 测量文本。 |
| `clip_set` / `clip_clear` | 裁剪。 |
| `request_refresh` | 请求宿主刷新。 |

### `xge_xui_style_t` / `xge_xui_theme_t`

Style 保存布局、dock、尺寸、margin、padding、anchor、grid、align、z、clip、background、radius、border、focus ring、disabled overlay 和 debug outline。Theme 保存默认字体、颜色、状态色、圆角、padding、spacing 和 border width。

## 主题与内置 Tech Blue 风格

XUI 默认主题第一版采用 `Tech Blue`：淡蓝科技风、扁平、线框与浅色块，不依赖图片素材。默认背景为浅蓝灰，panel 接近白色，accent 使用清晰蓝色，容器外边框要明显强于标题栏分隔线和表格内部网格线。

### 主题 token 映射

| 语义 token | 当前字段 | 用途 |
| --- | --- | --- |
| `color.text.primary` | `iTextColor` | 主文字、图标默认 tint。 |
| `color.background` | `iBackgroundColor` | 页面和窗口底色。 |
| `color.panel` | `iPanelColor` | Panel、Window、MsgBox、Toolbar 和数据控件表面。 |
| `color.border.strong` | `iBorderColor` | 控件边框、容器外轮廓、可见分割边。 |
| `color.accent` | `iAccentColor` | 主按钮、选中态、开关开启态、重要焦点。 |
| `color.selection` | `iSelectionColor` | 文本选择、列表选中、hover 淡色背景。 |
| `state.normal` | `iStateNormal` | Button、Input、Tab 等默认填充。 |
| `state.hover` | `iStateHover` | 鼠标悬停填充。 |
| `state.active` | `iStateActive` | 按下、拖动中或强交互态填充。 |
| `state.focus` | `iStateFocus` | focus ring、输入光标附近的强调描边。 |
| `state.disabled` | `iStateDisabled` | 禁用填充或禁用遮罩。 |
| `size.radius` | `fRadius` | 默认圆角，第一版为 5px。 |
| `size.padding` | `fPadding` | 控件内边距，第一版为 6px。 |
| `size.gap` | `fSpacing` | 控件内部和相邻项间距，第一版为 6px。 |
| `size.border` | `fBorderWidth` | 标准描边宽度，第一版为 1.5px。 |
| `font.ui.body` | `pFont` | 默认 UI 字体；Windows 优先宋体 9pt，按 12px 加载。 |

`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont` 和 `xgeXuiTokenSetTexture` 用于注册 context 级 token。XSON/import token 优先，context token 作为 fallback；font/texture token 只保存外部资源指针，不接管生命周期。

### 尺寸与字体

标准工具型控件默认高度优先按 24px 校准，小型控件 20px，大型控件 28px 到 32px。默认边框 1px 到 1.5px，圆角 4px 到 6px，水平 padding 8px，图标与文字 gap 6px。Button、Input、List row、StatusBar 都应以 9pt UI 字体的清晰 baseline 为准，避免为了视觉居中牺牲小字号可读性。

字体 token 第一版约定为 `font.ui.small`、`font.ui.body`、`font.ui.mono`。Windows 默认 UI 字体优先加载 `simsun.ttc`，用于获得小字号下更清楚的中文边缘；加载失败时回退到微软雅黑、黑体、Arial 等可用字体。

### 状态与层级

基础状态色遵循 `normal -> hover -> active -> focus -> disabled` 的递进。focus ring 使用 `state.focus`，绘制在控件边框内侧或紧贴外侧，宽度保持 1px 到 1.5px；禁用控件不绘制 focus ring。error、loading、dirty 为语义叠加状态，不替代 hover/focus，例如 error input 在 focus 时仍保留 focus 提示。

输入控件 error 状态应同时具备边框、浅色背景、提示文本和图标预留。error 边框优先使用低饱和红色，背景只做轻微 tint，避免破坏 Tech Blue 主调。

Overlay surfaces use light panels, clear borders, compact title/content layers and z-order. Menu, ComboBox, MsgBox, InputBox, Window, Toast and Tooltip share this visual language.

### 控件视觉约定

Buttons, inputs, choice controls, menus, popups, MsgBox, InputBox, Window, Tooltip and Toast share the same XUI visual language: light panels, clear borders, compact spacing, and widget-driven state colors.

Toast 的 `backgroundColor`/`borderColor` 应用于每条 toast item surface，不用于填充整个 overlay widget。overlay widget 本体保持透明，只负责队列布局、事件命中、过期更新和裁剪。

没有 texture host 或不提供图片资源时，控件必须退回几何绘制或内置 bitmap mask：check、radio ring、radio dot、switch knob、triangle、chevron、close 等基础符号都由代码内数据或 shape primitive 绘制，并按当前文字色、accent 或控件状态色 tint。

## Overlay Policy

Overlay Policy unifies Popup, Menu, ComboBox, MsgBox, InputBox, Window, Toast and widget tooltip layers: owner, open/close policy, placement, focus restore and z order. Popup details are documented in [Popup](../xui/popup.md).

XSON retained pages attach regular nodes to the active XUI root. Structural overlay controls such as `popup` are attached to the overlay root through an overlay portal. Service overlays such as `msgTip`、`msgBox` and `inputBox` are opened by C API callbacks and are not XSON page nodes.

Tooltip API：

```c
void xgeXuiWidgetSetTooltipText(xge_xui_widget widget, const char* text);
void xgeXuiWidgetSetTooltip(xge_xui_widget widget, const xge_xui_tooltip_desc_t* desc);
void xgeXuiWidgetClearTooltip(xge_xui_widget widget);
const xge_xui_tooltip_desc_t* xgeXuiWidgetGetTooltip(xge_xui_widget widget);
int xgeXuiWidgetTooltipIsOpen(xge_xui_context context);
xge_xui_widget xgeXuiWidgetTooltipGetOwner(xge_xui_context context);
xge_rect_t xgeXuiWidgetTooltipGetRect(xge_xui_context context);
```

`xge_xui_tooltip_desc_t` 支持 `TEXT` 和 `CUSTOM` 两种类型。文本提示由 XUI 默认绘制；自定义提示通过 `measure/paint` 回调提供尺寸和内容。定位支持 widget bottom/top/right/left 和 cursor，带 offset、delay、follow cursor。

XSON 加载时，`tooltip` 是任意 widget 的通用属性，不是独立控件实例。支持字符串简写：`"tooltip":"Apply changes"`；也支持对象配置：`"tooltip":{"text":"Apply changes","anchor":"right","offsetX":4,"offsetY":2,"delay":0,"followCursor":false}`。`tooltip:null`、`tooltip:false` 或对象内 `enabled:false` 会清除该 widget 的提示。XSON 不支持 `type:"tooltip"` 节点。

### 基础对象

- `owner widget`：触发 overlay 的控件。owner 区域点击默认穿透；owner hidden 或 disabled 时，打开的 Popup 会自动关闭。
- `popup widget`：挂到 `xgeXuiOverlayRoot(context)` 下的浮层 widget。overlay root 独立于 root widget 绘制和命中，叠放按 layer、zIndex、treeOrder 统一排序。
- `open flag`：`xgeXuiPopupSetOpen` 控制可见性和焦点。打开时记录 focus restore，关闭时优先恢复到指定 focus restore widget。
- `modal flag`：Popup modal 时，outside click 即使不自动关闭也会消费事件，避免穿透到底层内容。
- `close policy`：Popup 关闭行为由 `xgeXuiPopupSetClosePolicy` 表达，分别控制 outside click、owner click 和 Escape。

### Placement

Placement 常量：

```c
XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT
XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_RIGHT
XGE_XUI_OVERLAY_PLACEMENT_TOP_LEFT
XGE_XUI_OVERLAY_PLACEMENT_TOP_RIGHT
XGE_XUI_OVERLAY_PLACEMENT_RIGHT_TOP
XGE_XUI_OVERLAY_PLACEMENT_LEFT_TOP
XGE_XUI_OVERLAY_PLACEMENT_CENTER
XGE_XUI_OVERLAY_PLACEMENT_CURSOR
XGE_XUI_OVERLAY_PLACEMENT_MANUAL
```

`xgeXuiPopupSetAnchorRect` 设置 anchor，未设置时使用 owner rect。`xgeXuiPopupSetOffset` 设置偏移。旧 placement 口径不作为新实现入口；新 Popup 使用 `anchorPoint + direction` 表达弹出位置。新设计不缩放内容尺寸；窗口放不下时，Popup 缩小 viewport 并启用内部滚动。`MANUAL` 不自动改动 rect，适合调用方已经完成定位的特殊浮层。

### API

```c
XGE_API int xgeXuiOverlayAttach(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_widget pOwner, int iLayer);
XGE_API void xgeXuiOverlayDetach(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiOverlayBringToFront(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiOverlayGetOwner(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiOverlayTop(xge_xui_context pContext);

XGE_API void xgeXuiPopupSetModal(xge_xui_popup pPopup, int bModal);
XGE_API void xgeXuiPopupSetClosePolicy(xge_xui_popup pPopup, int iOutsidePolicy, int iOwnerPolicy, int iEscapePolicy);
XGE_API void xgeXuiPopupSetFocusPolicy(xge_xui_popup pPopup, int iFocusPolicy, xge_xui_widget pCustomFocus);
XGE_API void xgeXuiPopupSetPlacement(xge_xui_popup pPopup, int iPlacement);
XGE_API void xgeXuiPopupSetAnchorRect(xge_xui_popup pPopup, xge_rect_t tAnchor);
XGE_API void xgeXuiPopupSetOffset(xge_xui_popup pPopup, float fX, float fY);
XGE_API void xgeXuiPopupSetFocusRestore(xge_xui_popup pPopup, xge_xui_widget pWidget);
XGE_API void xgeXuiPopupSetContentWidget(xge_xui_popup pPopup, xge_xui_widget pContent);
XGE_API void xgeXuiPopupSetContentSize(xge_xui_popup pPopup, float fW, float fH);
XGE_API xge_rect_t xgeXuiPopupGetViewportRect(xge_xui_popup pPopup);
XGE_API xge_rect_t xgeXuiPopupGetContentRect(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetScroll(xge_xui_popup pPopup, float fX, float fY);
XGE_API void xgeXuiPopupGetScroll(xge_xui_popup pPopup, float* pX, float* pY);
XGE_API void xgeXuiPopupApplyPlacement(xge_xui_popup pPopup);
```

`xgeXuiOverlayTop` 返回 overlay root 下当前可见且排序最高的 widget。默认排序由 layer 决定大类优先级，同 layer 下先比较 zIndex，再比较 treeOrder；`xgeXuiOverlayBringToFront` 会更新 treeOrder。需要手动调整同层顺序时，直接对 overlay widget 调用 `xgeXuiWidgetSetZ`。

## 函数分组

| 分组 | 函数 |
| --- | --- |
| Size | `xgeXuiSizePx`、`xgeXuiSizeDip`、`xgeXuiSizePercent`、`xgeXuiSizeGrow`、`xgeXuiSizeContent` |
| Context | `xgeXuiInit`、`xgeXuiUnit`、`xgeXuiRoot` |
| DIP / Safe Area | `xgeXuiSetDipScale`、`xgeXuiGetDipScale`、`xgeXuiSetSafeAreaPx` |
| Theme/Style | `xgeXuiThemeDefault`、`xgeXuiSetTheme`、`xgeXuiGetTheme`、`xgeXuiGetThemeVersion`、`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`、`xgeXuiStyleDefault`、`xgeXuiStyleFromTheme` |
| Refresh/Layout Batch | `xgeXuiRefreshRequest`、`xgeXuiRefreshNeeded`、`xgeXuiRefreshClear`、`xgeXuiLayoutBatchBegin`、`xgeXuiLayoutBatchEnd` |
| Host | `xgeXuiSetHost`、`xgeXuiGetHost` |
| Page / Loader | `xgeXuiPageLoad`、`xgeXuiPageLoadMemory`、`xgeXuiPageUnload`、`xgeXuiPageRoot`、`xgeXuiPageFind`、`xgeXuiPageStyleVersion`、`xgeXuiPageGetError` |
| Binder | `xgeXuiBinderInit`、`xgeXuiBinderSetClick` |
| Widget | `xgeXuiWidget*`、`xgeXuiHitTest`、`xgeXuiSetFocus`、`xgeXuiSetCapture`、`xgeXuiSetPointerCapture`、`xgeXuiReleaseWidgetCapture` |
| Event | `xgeXuiDispatchEvent`、`xgeXuiEventPush`、`xgeXuiEventPop`、`xgeXuiEventCount`、`xgeXuiDispatchQueuedEvents` |
| Frame | `xgeXuiUpdate`、`xgeXuiPaint` |
| Text | `xgeXuiText*` |
| Viewport Infra | `xgeXuiScrollModel*`、`xgeXuiScrollFrame*`、`xgeXuiScrollBar*` |
| Controls | `xgeXuiButton*`、`xgeXuiLabel*`、`xgeXuiImage*`、`xgeXuiInput*`、`xgeXuiSlider*`、`xgeXuiProgress*`、`xgeXuiPanel*`、`xgeXuiScrollView*`、`xgeXuiPopup*`、`xgeXuiListView*`、`xgeXuiTreeView*`、`xgeXuiTableView*`、`xgeXuiTableGrid*`、`xgeXuiNumericInput*`、`xgeXuiColorPicker*`、`xgeXuiDatePicker*`、`xgeXuiToolbar*`、`xgeXuiStatusBar*`、`xgeXuiAccordion*` |
| Overlay Services | `xgeXuiMsgTip*`、`xgeXuiMsgBox*`、`xgeXuiInputBox*`、`xgeXuiToast*` |
| Quarantined Viewport Controls | `xgeXuiVirtualList*`、`xgeXuiPropertyGrid*` |

> 本页 API 数量较多，采用分批展开。当前已展开 Size / Context / Theme / Host 核心函数；Widget、Event、Text 和 Controls 会在后续批次继续补齐。

## 新增控件概览

| 控件 | 定位 | 示例 |
| --- | --- | --- |
| `NumericInput` | 带上下调节按钮、范围、步进和格式化的数值输入。 | `examples/xui_numericinput` / `examples/xui_numericinput_xson` |
| `MsgBox` / `InputBox` | Window-based modal or non-modal message and text input boxes. | `examples/xui_msgbox_inputbox` |
| `MsgTip` | 单条轻量提示服务，支持类型、自动过期、点击关闭、换行和自定义颜色；不作为 XSON 页面节点。 | `examples/xui_msgtip` |
| `Toolbar` | 水平/垂直工具条，支持普通项、toggle 项、分隔项、分组、tooltip 元数据、overflow 按钮和禁用态。 | `examples/xui_toolbar_lab` |
| `StatusBar` | 底部状态条，支持文本、进度、固定/弹性 spacer 和可点击状态项。 | `examples/xui_status_bar_lab` |
| `TreeView` | 层级树，支持展开、折叠、选择、勾选、禁用节点、键盘导航和自定义行绘制。 | `examples/xui_treeview` / `examples/xui_treeview_xson` |
| `TableView` | 静态表格视图，支持列宽、行/单元格选择、横纵滚动、合并单元格、自定义渲染和 XSON 静态数据。 | `examples/xui_tableview` / `examples/xui_tableview_xson` |
| `TableGrid` | 可编辑表格层，复用 TableView 的表格基础能力，支持 text、int、float、bool、textarea、enum、color、date/time/datetime、picker/file/image/custom 编辑入口和 XSON 静态数据。 | `examples/xui_tablegrid` / `examples/xui_tablegrid_xson` |
| `PropertyGrid` | 属性编辑表，支持分类、只读、已改动和错误状态。 | `examples/xui_property_grid_lab` |
| `Accordion` | 折叠面板，支持 single/multiple 展开模式。 | `examples/xui_accordion_lab` |
| `ColorPicker` | 颜色选择控件，支持 swatch、RGBA 字段、hex、palette 和键盘编辑。 | `examples/xui_colorpicker` / `examples/xui_colorpicker_xson` |
| `DatePicker` | 表单化日期时间选择控件，底层一个控件通过 mode 支持日期、时间、日期时间和三类范围选择。 | `examples/xui_datepicker` / `examples/xui_datepicker_xson` |
| `Toast` | 轻量通知队列，支持类型、关闭、过期和屏幕位置。 | `examples/xui_toast_lab` |
| `DockLayout` / `DockWindow` | DockPanelSuite 风格停靠面板，支持 region、pane tab、split tree、XUI root 内 floating dockwindow、drag indicator 和 preview rect。 | `examples/xui_dockpanel_lab` / `examples/xui_dockpanel_xson` |

上表控件支持在 XGE host 下绘制。仍处于隔离状态的 viewport 依赖控件恢复时以 `ScrollModel + ScrollFrame + ScrollView/VirtualView` 为准。

### DockPanel / DockLayout

DockPanel 由 `xge_xui_dock_layout_t` 管理，内部模型为 `docklayout -> dock_region -> dock_node(split|pane) -> dock_pane -> dockwindow tabs`。`dock_node` 使用显式二叉 split tree，便于 C 结构维护、调试和后续序列化。

`xge_xui_dock_window_t` 不复制一套普通 window，而是组合现有 `xge_xui_window_t`。停靠时普通 window 外框关闭，dockpane 负责绘制 tab、title、pane button 和 client slot；浮动时恢复内部 `xge_xui_window_t` 的特殊 dockwindow 外观，并将 rect clamp 在当前 XUI root 工作区内，不创建 OS/native 子窗口。

基础调用顺序：

```c
xge_xui_widget layoutWidget = xgeXuiWidgetCreate();
xge_xui_dock_layout_t layout;
xge_xui_dock_window_t doc;

xgeXuiDockLayoutInit(&layout, &xui, layoutWidget);
xgeXuiDockWindowInit(&doc, &xui);
xgeXuiDockWindowSetTitle(&doc, "Document.c");
xgeXuiDockWindowSetClosable(&doc, 0);
xgeXuiDockWindowSetDockable(&doc, 1);
xgeXuiDockWindowSetClientWidget(&doc, editorWidget);
xgeXuiDockLayoutDockWindow(&layout, &doc, XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
```

公开 API：

```c
XGE_API int xgeXuiDockLayoutInit(xge_xui_dock_layout pLayout, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiDockLayoutUnit(xge_xui_dock_layout pLayout);
XGE_API xge_xui_widget xgeXuiDockLayoutWidget(xge_xui_dock_layout pLayout);
XGE_API xge_xui_dock_pane xgeXuiDockLayoutDockWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, int iRegion, int iSide, float fProportion);
XGE_API int xgeXuiDockLayoutFloatWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_rect_t tRect);
XGE_API int xgeXuiDockLayoutHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockLayoutDockAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
XGE_API void xgeXuiDockLayoutSetRegionPortion(xge_xui_dock_layout pLayout, int iRegion, float fPortion);
XGE_API xvalue xgeXuiDockLayoutSaveState(const xge_xui_dock_layout pLayout);
XGE_API int xgeXuiDockLayoutLoadState(xge_xui_dock_layout pLayout, xvalue pState);
XGE_API void xgeXuiDockLayoutStateFree(xvalue pState);
XGE_API int xgeXuiDockLayoutStateGetCounts(xvalue pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount);
XGE_API int xgeXuiDockWindowInit(xge_xui_dock_window pWindow, xge_xui_context pContext);
XGE_API void xgeXuiDockWindowUnit(xge_xui_dock_window pWindow);
XGE_API xge_xui_window xgeXuiDockWindowBaseWindow(xge_xui_dock_window pWindow);
XGE_API xge_xui_widget xgeXuiDockWindowClientWidget(xge_xui_dock_window pWindow);
XGE_API void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window pWindow, xge_xui_widget pClient);
XGE_API void xgeXuiDockWindowSetTitle(xge_xui_dock_window pWindow, const char* sTitle);
XGE_API void xgeXuiDockWindowSetIcon(xge_xui_dock_window pWindow, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiDockWindowSetClosable(xge_xui_dock_window pWindow, int bClosable);
XGE_API void xgeXuiDockWindowSetDockable(xge_xui_dock_window pWindow, int bDockable);
XGE_API int xgeXuiDockWindowGetState(const xge_xui_dock_window pWindow);
XGE_API int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane pPane);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetWindow(const xge_xui_dock_pane pPane, int iIndex);
XGE_API xge_xui_dock_window xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane pPane);
XGE_API void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane pPane, int iIndex);
XGE_API int xgeXuiDockPaneGetActiveIndex(const xge_xui_dock_pane pPane);
```

region 和 side 常量：

```c
XGE_XUI_DOCK_REGION_DOCUMENT
XGE_XUI_DOCK_REGION_LEFT
XGE_XUI_DOCK_REGION_RIGHT
XGE_XUI_DOCK_REGION_TOP
XGE_XUI_DOCK_REGION_BOTTOM

XGE_XUI_DOCK_SIDE_FILL
XGE_XUI_DOCK_SIDE_LEFT
XGE_XUI_DOCK_SIDE_RIGHT
XGE_XUI_DOCK_SIDE_TOP
XGE_XUI_DOCK_SIDE_BOTTOM
```

状态与所有权：

- `xgeXuiDockWindowSetClientWidget` 只保存并 reparent 调用方传入的 client widget，不接管 widget 内存所有权。
- `xgeXuiDockLayoutDockWindow` 将 dockwindow 切换到 `XGE_XUI_DOCK_WINDOW_DOCKED`；fill 停靠加入目标 pane tab，side 停靠拆分 pane/region。
- `xgeXuiDockLayoutFloatWindow` 将 dockwindow 切换到 `XGE_XUI_DOCK_WINDOW_FLOATING`，client widget 挂回内部 `xge_xui_window_t` client slot。
- `xgeXuiDockLayoutHideWindow` 将 dockwindow 切换到 `XGE_XUI_DOCK_WINDOW_HIDDEN`，从 pane/floating 列表移除并保留可恢复状态。
- `xgeXuiDockLayoutAutoHideWindow` 将可停靠 dockwindow 收入边缘 strip；点击 strip 会打开临时 overlay pane，不修改原 split tree。overlay 内 dock 按钮调用 `xgeXuiDockLayoutDockAutoHideWindow` 恢复到保存的 region/side，close、outside click 或 Escape 会收回到 strip。
- `xgeXuiDockWindowUnit` 会先从所属 layout/pane/floating 列表脱离，再释放内部 window 资源；调用方仍负责释放传入的 client widget。
- `xgeXuiDockLayoutSaveState` 返回 caller-owned XValue table，保存 region、split tree、pane tab ids、floating rect 和 dockwindow 状态；外部调用方用 `xgeXuiDockLayoutStateFree` 释放。
- `xgeXuiDockLayoutLoadState` 从 XValue table 恢复 region、split tree、pane tabs、floating rect 和 hidden 状态。它只解析当前已注册到该 layout 的 dockwindow，按 widget name 优先、title 兜底的保存 id 匹配；未知或重复 id 会返回错误并保持原 layout 不变。

交互边界：

- dock drag 接入 XUI event、pointer capture、overlay root 和 `XGE_XUI_LAYER_DRAG_ADORNER`，不依赖普通 widget hit-test 决定 drop target。
- 拖拽时显示 DockPanelSuite VS2005 风格 pane diamond / panel indicator 和 preview rect。
- `Escape` 取消 dock drag 或 splitter drag；拖拽期间 `Tab` 不穿透到普通 focus step。
- drag move 带 `XGE_KEY_MOD_CTRL` 时临时抑制 docking，只保留 floating preview。
- pane option menu 提供 Float、Dock 占位、Close、Close Others、Close All；tab overflow menu 在 tab 放不下时列出当前 pane 的全部 tab。

XSON 声明：

```json
{
  "type": "dockLayout",
  "id": "mainDock",
  "regions": { "left": 0.22, "right": 0.2, "bottom": 0.25 },
  "dockWindows": [
    {
      "id": "document",
      "title": "Document",
      "region": "document",
      "side": "fill",
      "closable": false,
      "children": [ { "type": "label", "text": "Document client" } ]
    },
    {
      "id": "toolbox",
      "title": "Toolbox",
      "region": "left",
      "portion": 0.22,
      "dockable": false,
      "children": [ { "type": "label", "text": "Toolbox" } ]
    },
    {
      "id": "preview",
      "title": "Preview",
      "state": "floating",
      "rect": [560, 74, 260, 170]
    }
  ]
}
```

`dockWindows` 支持 `state:"docked"`、`state:"floating"` 和 `state:"hidden"`；`region` 可为 `document/left/right/top/bottom`，`side` 可为 `fill/left/right/top/bottom`。声明式 children 会挂入对应 dockwindow client，page unload 时由 page control arena 释放。

当前限制：

- 第一版不支持拖出 XUI root 形成 OS/native window。
- XSON `state:"autoHide"` 会创建收起到 strip 的 dockwindow，并复用交互式 auto-hide overlay 展开路径；应用层持久化文件格式和显式二叉 split tree 的 XSON roundtrip 仍在后续阶段，运行时 XValue save/load 已可用于应用自行保存配置。

### 新增控件 API 速查

仍处于隔离重构的 viewport 相关控件不列入当前可用口径。新的公开入口以 [Viewport / Scroll](../xui/scrollview.md) 和对应控件文档为准。

| 控件 | 创建/释放 | 常用状态与数据 | 事件/绘制入口 |
| --- | --- | --- | --- |
| `ColorPicker` | `xgeXuiColorPickerInit` / `xgeXuiColorPickerUnit` | `SetChange`、`SetColor`、`GetColor`、`SetRGBA`、`GetRGBA`、`SetAlphaEnabled`、`GetAlphaEnabled`、`SetHex`、`GetHex`、`SetPalette`、`GetPaletteCount`、`SetColors`、`IsPopupOpen` | `Event`、`EventProc`、`PaintProc` |
| `DatePicker` | `xgeXuiDatePickerInit` / `xgeXuiDatePickerUnit` | `SetMode`、`SetValue`、`GetValue`、`SetRangeValue`、`GetRangeValue`、`SetLimits`、`SetFormat`、`SetRangeSeparator`、`SetShowSecond`、`SetDefaultRangeSpan`、`SetChange`、`SetCommit`、`SetCancel`、`SetClear` | `Event`、`EventProc`、`PaintProc` |
| `NumericInput` | `xgeXuiNumericInputInit` / `xgeXuiNumericInputUnit` | `SetChange`、`SetFormatter`、`SetRange`、`SetStep`、`SetInteger`、`SetPrecision`、`SetSpinnerVisible`、`SetSpinnerWidth`、`SetSpinnerColors`、`SetValue`、`GetValue`、`GetState` | `Event`、`EventProc`、`UpdateProc`、`PaintProc` |
| `Toolbar` | `xgeXuiToolbarInit` / `xgeXuiToolbarUnit` | `SetItems`、`SetFont`、`SetSelect`、`SetOrientation`、`SetItemSize`、`SetGroupGap`、`SetItemGroup`、`SetItemTooltip`、`SetOverflow`、`SetItemEnabled`、`SetItemChecked`、`GetState` | `Event`、`EventProc`、`PaintProc` |
| `StatusBar` | `xgeXuiStatusBarInit` / `xgeXuiStatusBarUnit` | `Clear`、`AddText`、`AddProgress`、`AddSpacer`、`AddFlexibleSpacer`、`SetFont`、`SetSelect`、`SetItemEnabled`、`SetItemText`、`SetProgress`、`SetMetrics`、`SetColors`、`GetState` | `Event`、`EventProc`、`PaintProc` |
| `Accordion` | `xgeXuiAccordionInit` / `xgeXuiAccordionUnit` | `Clear`、`AddSection`、`GetSectionCount`、`IsExpanded`、`SetExpanded`、`SetMode`、`SetFont`、`SetMetrics`、`SetSelect`、`SetColors`、`GetContentHeight` | `Event`、`EventProc`、`PaintProc` |
| `Toast` | `xgeXuiToastInit` / `xgeXuiToastUnit` | `Clear`、`Show`、`Close`、`GetCount`、`SetPlacement`、`SetMetrics`、`SetColors` | `Event`、`EventProc`、`UpdateProc`、`PaintProc` |
| `MsgBox` | `xgeXuiMsgBoxInit` / `xgeXuiMsgBoxUnit` | `SetText`, `SetType`, `SetIconTexture`, `SetButtons`, `SetCustomButtons`, `SetResult`, `SetModal`, `SetOpen`, `IsOpen`, `GetResult`, `SetColors` | `Event`, `EventProc`, `PaintProc` |
| `InputBox` | `xgeXuiInputBoxInit` / `xgeXuiInputBoxUnit` | `SetText`, `SetResult`, `SetModal`, `SetOpen`, `IsOpen`, `GetResultCode`, `GetResult`, `SetColors` | `Event`, `EventProc`, `PaintProc` |
| `ListView` | `xgeXuiListViewInit` / `xgeXuiListViewUnit` | `SetItems`、`SetEnabledItems`、`SetItemHeight`、`SetSelected`、`SetSelectionMode`、`SetSelectionBuffer`、`SetItemRenderer`、`SetScrollbarMode`、`SetColors` | `Event`、`EventProc`、`PaintProc` |
| `TreeView` | `xgeXuiTreeViewInit` / `xgeXuiTreeViewUnit` | `Clear`、`AddNode`、`SetAdapter`、`RefreshAdapter`、`SetNodeExpanded`、`SetNodeEnabled`、`SetNodeChecked`、`SetNodeDecorations`、`SetSelected`、`SetItemRenderer`、`SetScrollbarMode`、`SetColors` | `Event`、`EventProc`、`PaintProc` |
| `TableView` | `xgeXuiTableViewInit` / `xgeXuiTableViewUnit` | `SetColumns`、`SetRows`、`SetAdapter`、`SetSelectionMode`、`SetSelectedRow`、`SetSelectedCell`、`SetMergeProvider`、`SetHeaderRenderer`、`SetCellRenderer`、`SetColumnFormatter`、`SetHover`、`SetScrollbarMode`、`SetDefaultMetrics`、`GetActiveCell`、`GetCellRect`、`GetCellContentRect`、`GetBodyWidget`、`GetViewportWidget`、`EnsureCellVisible` | `Event`、`EventProc`、`PaintProc` |
| `TableGrid` | `xgeXuiTableGridInit` / `xgeXuiTableGridUnit` | `GetTableView`、`SetColumns`、`SetRows`、`SetAdapter`、`SetValidate`、`SetChange`、`SetEditor`、`SetEditorConfig`、`SetEditMode`、`SetFont`、`SetDefaultMetrics`、`SetSelectionMode`、`SetScrollbarMode`、`SetColors`、`BeginEdit`、`EndEdit`、`IsEditing`、`GetEditingCell` | `Event`、`EventProc` |

实现文件中的新增控件公开函数已与 `xge.h` 声明比对；新增控件源文件也已全部纳入 `src/xge_impl.c` 的统一编译入口。

ColorPicker 已按 [ColorPicker](../xui/colorpicker.md) 和 [Popup](../xui/popup.md) 恢复。它接入新的 Popup/ScrollFrame 路径，弹层在每个 `xge_xui_context` 内复用唯一共享 panel；实例只保存打开时的活动 popup widget 引用，不再持有独立 popup 副本。

搜索框不再作为 core 控件提供。搜索输入应使用 `input` 加 leading search decoration、trailing clear decoration 和 Submit 行为组合；带 suggestion popup 的搜索框后续放入 ext/组合控件。

DatePicker 是单 widget 表单控件：通过 `mode` 切换 `date`、`time`、`dateTime`、`dateRange`、`timeRange`、`dateTimeRange`。值统一使用 `xtime`，单值走 `SetValue`/`GetValue`，范围走 `SetRangeValue`/`GetRangeValue`；范围默认自动交换起止值，半开草稿提交时按默认跨度补齐。Popup 使用通用 Popup/ScrollFrame 路径，并在每个 `xge_xui_context` 内复用 6 个共享 DatePicker popup panel；弹层内部复用标准 `Button`、`Input`、`ComboBox`、`NumericInput`，只有日历网格由 DatePicker 自绘。输入框不可手输，只通过弹层选择。XSON 中使用 `type: "datePicker"`，支持 `mode`、`value`、`startValue`、`endValue`、`min`、`max`、`nullable`、`showSecond`、`format`、`rangeSeparator`、`defaultRangeSpan` 和颜色字段。

TreeView 已按 [TreeView](../xui/treeview.md) 恢复。

TableView 已按 [TableView](../xui/tableview.md) 恢复，旧 `virtual_scroll_view_base` 口径已移除。

TableGrid 已按 [TableGrid](../xui/tablegrid.md) 恢复：它复用 TableView，提供 text、int、float、bool、textarea、enum、color、date/time/datetime、picker/file/image/custom 的编辑入口，并支持 XSON `tableGrid` 静态数据。`XGE_XUI_TABLE_GRID_EDIT_IMMEDIATE` 和 custom 编辑器完整生命周期仍按 [TableGrid Spec](../xui/tablegrid-spec.md) 跟踪。

PropertyGrid 后续恢复时必须复用新 viewport 基础设施。

## API 参考：Size / Context / Host

### xgeXuiSizePx

创建像素尺寸描述。

**功能：**

你可以用它声明一个不受 DIP 缩放影响的固定像素尺寸。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
```

**参数：**

- `fValue`：像素值。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_PX`、`fValue = fValue` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析时该值直接作为像素使用。

**范例代码：**

```c
style.tWidth = xgeXuiSizePx(240.0f);
```

**相关 API：**

- `xgeXuiSizeDip`
- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeDip

创建 DIP 尺寸描述。

**功能：**

你可以用它声明随 `xgeXuiSetDipScale` 缩放的界面尺寸。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeDip(float fValue);
```

**参数：**

- `fValue`：DIP 值。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_DIP` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析时会乘以当前 context 的 DIP scale。

**范例代码：**

```c
style.tHeight = xgeXuiSizeDip(44.0f);
```

**相关 API：**

- `xgeXuiSetDipScale`
- `xgeXuiGetDipScale`

---

### xgeXuiSizePercent

创建百分比尺寸描述。

**功能：**

你可以用它按父容器尺寸计算控件宽高。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
```

**参数：**

- `fValue`：百分比数值，例如 `50.0f` 表示 50%。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_PERCENT` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析公式为 `parent * fValue * 0.01f`。

**范例代码：**

```c
style.tWidth = xgeXuiSizePercent(100.0f);
```

**相关 API：**

- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeGrow

创建 grow 尺寸描述。

**功能：**

你可以用它让控件在 row/column/grid 等布局中使用剩余空间。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
```

**参数：**

- `fValue`：grow 权重。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_GROW` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

当前底层 size resolve 会把 grow 交给布局提供的 fallback 值。

**范例代码：**

```c
style.tWidth = xgeXuiSizeGrow(1.0f);
```

**相关 API：**

- `xgeXuiWidgetSetLayout`
- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeContent

创建内容自适应尺寸描述。

**功能：**

你可以用它让控件宽高优先来自 measure 回调或控件自身内容。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
```

**参数：**

无。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_CONTENT`、`fValue = 0` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

如果内容测量值不可用，布局会使用 fallback 尺寸。

**范例代码：**

```c
style.tWidth = xgeXuiSizeContent();
```

**相关 API：**

- `xgeXuiWidgetSetMeasure`
- `xgeXuiStyleDefault`

---

### xgeXuiInit

初始化 XUI context。

**功能：**

你可以用它创建 XUI root widget、默认主题、默认 host 和初始 dirty 状态。

**函数原型：**

```c
XGE_API int xgeXuiInit(xge_xui_context pContext);
```

**参数：**

- `pContext`：输出 XUI context，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- parent role 为 `XGE_XUI_WIDGET_ROLE_CONTROL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`；Control 不承载用户 children。
- context 已初始化返回 `XGE_ERROR_ALREADY_INITIALIZED`。
- root widget 分配失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

成功后 `pContext` 持有 root widget tree，调用者必须使用 `xgeXuiUnit` 释放。

**补充说明：**

- root 默认尺寸来自 `xgeGetWidth()` / `xgeGetHeight()`。
- 默认 DIP scale 为 `1.0f`。
- 默认 host 为 XGE 内置 host。

**范例代码：**

```c
xge_xui_context_t ui;
if (xgeXuiInit(&ui) == XGE_OK) {
    /* build UI */
}
```

**相关 API：**

- `xgeXuiUnit`
- `xgeXuiRoot`
- `xgeXuiSetHost`

---

### xgeXuiUnit

释放 XUI context。

**功能：**

你可以用它释放 root widget tree，并清零 context。

**函数原型：**

```c
XGE_API void xgeXuiUnit(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `xgeXuiInit` 创建的 root widget tree，不释放 context 结构体本身。

**补充说明：**

对 `NULL` 或未初始化 context 调用是安全的。

**范例代码：**

```c
xgeXuiUnit(&ui);
```

**相关 API：**

- `xgeXuiInit`

---

### xgeXuiRoot

获取 root widget。

**功能：**

你可以用它取得 context 的根节点，并把业务 widget 添加到 root 下。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效时返回 root widget。
- `pContext` 为 `NULL` 时返回 `NULL`。

**资源归属：**

返回值是借用指针，所有权属于 context，不能单独释放。

**补充说明：**

root 会随 `xgeXuiUnit` 释放。

**范例代码：**

```c
xge_xui_widget root = xgeXuiRoot(&ui);
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiUnit`

---

### xgeXuiSetDipScale

设置 DIP 缩放。

**功能：**

你可以用它根据窗口缩放、DPR 或 UI 缩放设置 DIP 到像素的转换比例。

**函数原型：**

```c
XGE_API void xgeXuiSetDipScale(xge_xui_context pContext, float fScale);
```

**参数：**

- `pContext`：XUI context。
- `fScale`：缩放值，小于等于 0 时按 `1.0f` 处理。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

scale 变化时会标记 root layout 和 paint dirty。

**范例代码：**

```c
xgeXuiSetDipScale(&ui, dpr);
```

**相关 API：**

- `xgeXuiGetDipScale`
- `xgeXuiSizeDip`

---

### xgeXuiGetDipScale

获取 DIP 缩放。

**功能：**

你可以用它读取当前 context 的 DIP scale。

**函数原型：**

```c
XGE_API float xgeXuiGetDipScale(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效且 scale 大于 0 时返回当前 scale。
- context 无效时返回 `1.0f`。

**资源归属：**

函数不分配资源。

**补充说明：**

布局和 paint 期间内部也会使用该值解析 DIP 尺寸。

**范例代码：**

```c
float scale = xgeXuiGetDipScale(&ui);
```

**相关 API：**

- `xgeXuiSetDipScale`

---

### xgeXuiSetSafeAreaPx

设置 XUI root 的安全区内边距。

**功能：**

root 和 overlay root 会自动跟随窗口尺寸。你可以用本函数让 root 的 content rect 避开刘海、系统栏或业务边距；overlay root 仍保持全窗口尺寸。

**函数原型：**

```c
XGE_API void xgeXuiSetSafeAreaPx(xge_xui_context pContext, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pContext`：XUI context。
- `fLeft` / `fTop` / `fRight` / `fBottom`：安全区内边距，单位为像素。

**返回值：**

无。

**相关 API：**

- `xgeXuiRoot`
- `xgeXuiWidgetSetPaddingPx`

**XSON：**

Page 顶层可声明 `safeArea`，加载时应用到当前 context root，卸载该 page 时恢复加载前的 root padding。`safeArea` 支持数字、二元数组 `[x, y]`、四元数组 `[left, top, right, bottom]`，并支持 spacing token：

```json
{
  "xui": 1,
  "tokens": { "spacing": { "safeX": 12, "safeY": 24 } },
  "safeArea": [ "@spacing.safeX", "@spacing.safeY" ],
  "tree": { "id": "root" }
}
```

---

### xgeXuiThemeDefault

生成默认主题。

**功能：**

你可以用它初始化一个主题结构，再按项目需要修改颜色、字体、圆角和间距。

**函数原型：**

```c
XGE_API void xgeXuiThemeDefault(xge_xui_theme pTheme);
```

**参数：**

- `pTheme`：输出主题，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只写入结构体字段，不接管字体资源。

**补充说明：**

默认主题使用 Tech Blue 浅色科技风：深蓝灰文字、浅蓝灰背景、接近白色的 panel、清晰蓝色 accent、淡蓝状态色、5px 圆角、6px padding/spacing 和 1.5px border width。`xgeXuiInit` 会在 context 初始化时尝试加载默认 UI 字体；Windows 优先宋体 9pt，失败时回退到其它清晰小字号字体。

**范例代码：**

```c
xge_xui_theme_t theme;
xgeXuiThemeDefault(&theme);
```

**相关 API：**

- `xgeXuiSetTheme`
- `xgeXuiStyleFromTheme`

---

### xgeXuiSetTheme

设置 context 主题。

**功能：**

你可以用它替换 XUI context 当前主题，并触发重新布局和重绘。

**函数原型：**

```c
XGE_API void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme);
XGE_API uint32_t xgeXuiGetThemeVersion(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。
- `pTheme`：主题输入；传 `NULL` 时恢复默认主题。

**返回值：**

无。

**资源归属：**

函数复制主题结构体，不接管 `pTheme` 或主题里的字体对象。

**补充说明：**

context 未初始化时调用无效果。`xgeXuiSetTheme` 每次成功设置都会递增 theme version，并标记 root layout/paint dirty；`xgeXuiGetThemeVersion` 返回当前版本。theme version 第一版用于后续 style/token cache 失效判断。

`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont` 分别注册 context 级颜色、间距和字体 token。注册成功后会递增 theme version，并标记 root style/layout/paint dirty。字体 token 只保存外部字体指针，不接管字体生命周期。

**范例代码：**

```c
xgeXuiSetTheme(&ui, &theme);
```

**相关 API：**

- `xgeXuiGetTheme`
- `xgeXuiThemeDefault`

---

### xgeXuiGetTheme

获取当前主题。

**功能：**

你可以用它读取 context 当前主题；context 无效时仍能取得一个静态默认主题。

**函数原型：**

```c
XGE_API const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

- context 有效时返回 `&pContext->tTheme`。
- context 无效时返回静态默认主题指针。

**资源归属：**

返回值是借用指针，调用者不能释放。

**补充说明：**

静态默认主题会在首次需要时初始化。

**范例代码：**

```c
const xge_xui_theme_t* theme = xgeXuiGetTheme(&ui);
```

**相关 API：**

- `xgeXuiSetTheme`
- `xgeXuiThemeDefault`

---

### xgeXuiStyleDefault

生成默认 style。

**功能：**

你可以用它初始化一个 widget style，再设置布局、尺寸、padding、颜色等字段。

**函数原型：**

```c
XGE_API void xgeXuiStyleDefault(xge_xui_style_t* pStyle);
```

**参数：**

- `pStyle`：输出 style，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只写入结构体字段，不分配资源。

**补充说明：**

默认 layout 为 absolute，dock 为 fill，width/height 为 content，align 为 stretch，背景透明，grid columns 为 1。

**范例代码：**

```c
xge_xui_style_t style;
xgeXuiStyleDefault(&style);
```

**相关 API：**

- `xgeXuiStyleFromTheme`
- `xgeXuiWidgetSetStyle`

---

### xgeXuiStyleFromTheme

从主题生成 style。

**功能：**

你可以用它基于主题快速生成 panel 风格的默认 style。

**函数原型：**

```c
XGE_API void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme);
```

**参数：**

- `pStyle`：输出 style，可以为 `NULL`。
- `pTheme`：输入主题；传 `NULL` 时使用默认主题。

**返回值：**

无。

**资源归属：**

函数只复制数值字段，不接管主题资源。

**补充说明：**

生成的 style 会使用主题 panel 色、圆角、padding 和 spacing。

**范例代码：**

```c
xge_xui_style_t style;
xgeXuiStyleFromTheme(&style, xgeXuiGetTheme(&ui));
```

**相关 API：**

- `xgeXuiThemeDefault`
- `xgeXuiWidgetSetStyle`

---

### xgeXuiRefreshRequest

请求刷新。

**功能：**

你可以用它标记 context 需要刷新，并通知 host 调度重绘。

**函数原型：**

```c
XGE_API void xgeXuiRefreshRequest(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果已经请求过刷新，函数会直接返回。host 提供 `request_refresh` 时会被调用一次。

**范例代码：**

```c
xgeXuiRefreshRequest(&ui);
```

**相关 API：**

- `xgeXuiRefreshNeeded`
- `xgeXuiRefreshClear`

---

### xgeXuiRefreshNeeded

查询是否需要刷新。

**功能：**

你可以在 APP 模式或手动刷新模式下判断是否需要调用 update/paint。

**函数原型：**

```c
XGE_API int xgeXuiRefreshNeeded(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 需要刷新返回 1。
- context 无效或无需刷新返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`xgeXuiPaint` 完成后会清除 refresh 请求。

**范例代码：**

```c
if (xgeXuiRefreshNeeded(&ui)) {
    xgeXuiUpdate(&ui, 0.0f);
    xgeXuiPaint(&ui);
}
```

**相关 API：**

- `xgeXuiRefreshRequest`
- `xgeXuiPaint`

---

### xgeXuiRefreshClear

清除刷新请求。

**功能：**

你可以手动清除 context 的 refresh 标记。

**函数原型：**

```c
XGE_API void xgeXuiRefreshClear(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

context 无效时调用无效果。

**范例代码：**

```c
xgeXuiRefreshClear(&ui);
```

**相关 API：**

- `xgeXuiRefreshNeeded`

---

### xgeXuiLayoutBatchBegin

开始布局批处理。

**功能：**

你可以在批量修改 widget 时调用它，延迟 dirty 传播和 refresh 请求，减少重复刷新。

**函数原型：**

```c
XGE_API void xgeXuiLayoutBatchBegin(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不分配资源。

**补充说明：**

批处理支持嵌套，内部通过 `iLayoutBatchDepth` 计数。

**范例代码：**

```c
xgeXuiLayoutBatchBegin(&ui);
```

**相关 API：**

- `xgeXuiLayoutBatchEnd`

---

### xgeXuiLayoutBatchEnd

结束布局批处理。

**功能：**

你可以用它结束一段批量修改；当批处理深度回到 0 时，XUI 会把累积 dirty 应用到 root 并请求刷新。

**函数原型：**

```c
XGE_API void xgeXuiLayoutBatchEnd(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果没有对应 begin 或 context 无效，函数直接返回。

**范例代码：**

```c
xgeXuiLayoutBatchEnd(&ui);
```

**相关 API：**

- `xgeXuiLayoutBatchBegin`
- `xgeXuiRefreshRequest`

---

### xgeXuiSetHost

设置 XUI host。

**功能：**

你可以用它替换 XUI 的绘制桥，接入 XGE 默认 host 或其他宿主框架。

**函数原型：**

```c
XGE_API void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost);
```

**参数：**

- `pContext`：XUI context。
- `pHost`：host 描述；传 `NULL` 时恢复默认 host。

**返回值：**

无。

**资源归属：**

函数只保存 host 借用指针，不复制 host，也不接管 `pUser`。

**补充说明：**

设置 host 后会标记 root paint dirty。

**范例代码：**

```c
xgeXuiSetHost(&ui, &host);
```

**相关 API：**

- `xgeXuiGetHost`
- `xgeXuiPaint`

---

### xgeXuiGetHost

获取当前 XUI host。

**功能：**

你可以用它取得当前绘制桥；context 无效或未设置 host 时返回默认 host。

**函数原型：**

```c
XGE_API const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

返回当前 host 或默认 host 的借用指针。

**资源归属：**

返回值不转移所有权，调用者不能释放。

**补充说明：**

XUI update/paint 期间会把该 host 设置为当前 active host。

**范例代码：**

```c
const xge_xui_host_t* host = xgeXuiGetHost(&ui);
```

**相关 API：**

- `xgeXuiSetHost`

---

## API 参考：Page / Loader / Binder

### xgeXuiPageLoad

从资源 URI 加载 XUI 页面。

**函数原型：**

```c
XGE_API int xgeXuiPageLoad(xge_xui_context pContext, const char* sURI, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

**说明：**

- 资源读取走 `xgeResourceLoad`。
XSON retained pages attach regular nodes to the active XUI root. Structural overlay controls such as `popup` are attached to the overlay root through an overlay portal. Service overlays such as `msgTip`、`msgBox` and `inputBox` are opened by C API callbacks and are not XSON page nodes.
- 已支持 `tree.type/id/name/children`、顶层 `styles`、widget `style` 引用、style `@parent` 继承，以及基础 layout/size/spacing/alignment/visual inline 覆盖字段。
- `tree.type` 第一版支持结构型控件：`panel`、`absolute`、`row`、`column`、`stack`、`grid`、`dock`、`scrollView`/`scroll`，并支持轻量状态控件：`button`、`image`、`input`、`label`、`separator`。未知类型或非字符串类型会使 page load 失败。
- 通用 widget/style 字段支持 `visible`、`enabled`、`overflow`、`layer`、`z`、`zIndex`、`hitTestVisible`、`inputTransparent`、`tabStop`、`tabIndex`、`imeMode`、`borderColor`、`borderWidth`、`focusRingColor`、`focusRingWidth`、`disabledOverlay`、`debugOutlineColor`、`debugOutlineWidth`；同级绘制、hit test 和事件目标选择统一使用 `layer > z > treeOrder`。
- `scrollView`/`scroll` 会绑定 `xge_xui_scroll_view_t`，内部由 `ScrollModel + ScrollFrame + contentWidget` 组成；XSON `children` 会挂到 content widget。支持 `contentSize`、`contentWidth`、`contentHeight`、`offset`/`scrollOffset`/`contentOffset`、`scrollX`、`scrollY`、`wheelAxis`、`wheelStep`、`dragMode`/`contentDrag`、`scrollbarPolicy`、`scrollbarPolicyX`、`scrollbarPolicyY`、`scrollbarMode`、`scrollbarSize`、`minThumbSize`、`thumbRadius`、`scrollbarButtonSize`、`backgroundColor`/`background`、`barColor`、`thumbColor`。滚动偏移会作用到 content widget 的布局结果，普通子控件不需要自行换算坐标。`button` 支持 `text`、`font`、`textColor`、`textAlign`、`textVAlign`、`color`/`background`、`hoverColor`、`activeColor`、`focusColor`、`disabledColor` 和 `onClick`。`button.onClick` 使用控件自身 `xgeXuiButtonSetClick`，不会覆盖控件 `pUser`。`image` 支持 `texture`、`src`、`source`/`srcRect`、`color`/`tint`、`mode`；`texture` 引用 C 侧注册的 texture token，`src` 由 page 同步加载并在 unload 时释放。`input` 支持 `text`/`value`、`placeholder`、`textAlign`、`font`、`textColor`、`background`/`backgroundColor`、`focusColor`、`cursorColor`、`placeholderColor`、`selectionColor`、`disabledTextColor`、`disabledBackgroundColor`、`password`、`readonly`、`disabled`、`selection`；文本缓冲、默认菜单和 IME 状态由 `xge_xui_input_t` 持有，随 page unload 调用 `xgeXuiInputUnit`。`label` 支持 `text`、`font`、`textColor`/`color`、`disabledColor`、`textAlign`、`textVAlign`、`underline`、`cacheMode`，其中 `font` 引用 C 侧注册的 font token，例如 `"@fonts.body"`，`cacheMode` 可为 `auto`、`off`、`force`。`separator` 支持 `orientation`、`thickness`、`color`/`background`。这些状态控件由 page 内固定容量 control arena 持有，并在 `xgeXuiPageUnload` 中调用对应 `Unit`。
- style 表直接使用 XValue table 父表链共享字段；父表不被单独持有，生命周期随 page document 统一释放。`@parent` 循环会导致 page load 失败，并通过 `xgeXuiPageGetError` 返回 `style parent cycle`。
- `imports` 第一版支持导入其它 XSON 资源中的 `styles`、`tokens`、`templates`，不会导入 `tree`。带 scheme 的 URI 原样走 `xgeResourceLoad`；相对路径按当前 XSON URI 所在目录解析。imports 按数组顺序合并，后导入覆盖先导入，当前 XSON 本地声明最终覆盖 imports。
- `tokens.colors` 可被颜色字段引用，例如 `"background": "@colors.panel"`；`tokens.spacing` 可被尺寸、间距和半径字段引用，例如 `"gap": "@spacing.md"`。未带 section 的 `@name` 会依次查找顶层 token、`colors`、`spacing`、`fonts`、`textures`。缺失 token 会使 page load 失败，并在错误中包含字段路径。C 侧可通过 `xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`、`xgeXuiTokenSetTexture` 注册 context 级 token；XSON/import token 优先，context token 作为 fallback。font/texture token 都只引用外部对象，不由 XSON 或 token 表释放。
- 尺寸字段支持数字 px、`"content"`、`"grow"`、`"grow:N"`、`"N%"`、`"Ndip"` 和 `{ "unit": "...", "value": N }`。`padding`/`margin` 支持数字、二元数组 `[x, y]`、四元数组 `[left, top, right, bottom]`。顶层 `safeArea` 使用同样的数字/数组格式，并在 page unload 时恢复加载前的 root padding。`anchor` 支持 `{ "left": N, "right": "5%" }` 这类对象，出现哪个边就启用哪个 anchor flag。Grid 支持 `grid.columns/rowHeight/columnGap/rowGap/columnSpan`，也支持顶层别名 `columns/rowHeight/columnGap/rowGap/columnSpan`。DockLayout 支持容器 `type:"dock"` 或 `layout:"dock"`，子元素 `dock` 可为 `top`、`bottom`、`left`、`right`、`fill`、`center`；布局按声明顺序扣减剩余区域，`fill/center` 使用当前剩余区域。ScrollView 示例：

```json
{
  "type": "scrollView",
  "width": 320,
  "height": 240,
  "padding": 8,
  "contentSize": [ 320, 800 ],
  "offset": { "x": 0, "y": 120 },
  "wheelAxis": "vertical",
  "dragMode": "none",
  "scrollbarPolicy": "auto",
  "scrollbarMode": "full",
  "children": [
    { "type": "column", "children": [] }
  ]
}
```
- `onClick` 支持绑定 C 侧 `xgeXuiBinderSetClick` 注册的事件名，例如 `"onClick": "ok"`。未注册事件名会使 page load 失败并报告字段路径。XSON 不支持脚本字段；`script`、`onClickScript` 会作为加载错误处理。结构型 widget 的 `onClick` 绑定到 page 创建的通用事件过程；`button` 的 `onClick` 绑定到按钮控件自身点击回调。`input.onChange/onSubmit` 当前严格拒绝，不提供空占位，后续随 model/value 绑定统一实现。

---

### xgeXuiPageLoadMemory

从内存加载 XUI 页面。

```c
XGE_API int xgeXuiPageLoadMemory(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

内存内容会复制进 page loader 持有的 resource，卸载时统一释放。

最小 XSON 示例：

```json
{
  "xui": 1,
  "imports": [ "shared/styles.xson" ],
  "styles": {
    "base": { "width": 120, "height": "grow:2", "padding": [8, 4] },
    "panel": { "@parent": "base", "layout": "row", "gap": 8 }
  },
  "tree": {
    "type": "column",
    "id": "root",
    "style": "panel",
    "children": [
      { "type": "row", "name": "child", "width": "50%" }
    ]
  }
}
```

---

### xgeXuiPageUnload / Root / Find / GetError

```c
XGE_API void xgeXuiPageUnload(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId);
XGE_API uint32_t xgeXuiPageStyleVersion(xge_xui_page_t* pPage);
XGE_API const char* xgeXuiPageGetError(xge_xui_page_t* pPage);
```

`xgeXuiPageUnload` 释放 loader 创建的 root、overlay portal widgets、XSON document、imports、合并后的 style/token/template 表、page control arena 中的状态控件和 resource。`xgeXuiPageFind` 优先使用 page load 时建立的固定容量 id/name 索引；索引溢出或手工填充 `pRoot` 且未建立索引时，会回退到 page root 与 page overlay portal 注册表下递归查找。page load 失败会回滚 loader 已创建资源、状态控件和 widget tree，并保留 `xgeXuiPageGetError` 可读取的错误字符串。`xgeXuiPageStyleVersion` 返回 page 当前 style version；第一版 page load 成功后为 `1`，用于后续 style/token cache 失效扩展。

context 级 token 变更会递增 theme version，但不会在布局或绘制热路径重新解析 XSON。需要让已加载 page 使用新的 token 值时，调用 `xgeXuiPageSyncStyle`；它只在 page 记录的 theme version 落后于 context 时调用 `xgeXuiPageRefreshStyle`。`xgeXuiPageRefreshStyle` 会重新遍历 page 的 XSON tree，把 layout/visual 字段解析回 widget 的 `xge_xui_style_t` cache；portaled overlay 节点会通过 page 内部 path 注册表找到实际 widget，递增 page style version，并标记受影响 widget 的 layout/paint dirty。

```c
XGE_API int xgeXuiPageRefreshStyle(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageSyncStyle(xge_xui_page_t* pPage);
```

---

### xgeXuiBinderInit / xgeXuiBinderSetClick

```c
XGE_API void xgeXuiBinderInit(xge_xui_binder_t* pBinder);
XGE_API int xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser);
```

Binder 第一版只提供事件名到 C 回调的注册容器。XSON 解析阶段会使用这些注册项绑定 `onClick`。

---

### XSON style 生命周期约束

`styles`、imports 中导入的 `styles`、以及 loader 内部合并后的 style table 都由 `xge_xui_page_t` 持有。`xvoTableSetParent` 只设置底层父表指针，不增加父表引用计数，也不接管父表所有权。因此第一版规则是：所有参与 `@parent` 查询的 XValue table 必须和 page 保持相同生命周期，并只能由 `xgeXuiPageUnload` 统一释放。

---

## API 参考：Widget / Event / Frame

### xgeXuiWidgetCreate

创建一个 widget。

**功能：**

你可以用它分配独立 widget 节点，之后再添加到 context root 或其他父节点下。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
```

**参数：**

无。

**返回值：**

- 成功返回新 widget。
- 内存不足返回 `NULL`。

**资源归属：**

返回 widget 由调用者持有，使用 `xgeXuiWidgetFree` 释放。

**补充说明：**

新 widget 默认 visible、enabled，并带 layout/paint dirty 标志。

**范例代码：**

```c
xge_xui_widget w = xgeXuiWidgetCreate();
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiWidgetFree`

---

### xgeXuiWidgetFree

释放 widget 子树。

**功能：**

你可以用它把 widget 从父节点移除，并递归释放其所有子节点。

**函数原型：**

```c
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：要释放的 widget，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 widget 树内由 XUI 分配的节点，不释放控件对象、font、texture 或用户数据。

**补充说明：**

对 `NULL` 调用安全。释放前会调用 `xgeXuiWidgetRemove`。

**范例代码：**

```c
xgeXuiWidgetFree(w);
```

**相关 API：**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetRemove`

---

### xgeXuiWidgetAdd

添加子 widget。

**功能：**

你可以用它把 child 挂到 parent 的子链表末尾。

**函数原型：**

```c
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
```

**参数：**

- `pParent`：父 widget，不能为 `NULL`。
- `pChild`：子 widget，不能为 `NULL`，且不能等于 parent。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

父节点只维护树关系，不接管业务控件对象。

**补充说明：**

如果 child 已有父节点，会先从旧父节点移除。添加后 parent 会标记 layout/paint dirty。Control 内部的 slot/组合结构由对应控件实现维护，不通过这个用户 children 入口暴露。

**范例代码：**

```c
xgeXuiWidgetAdd(xgeXuiRoot(&ui), w);
```

**相关 API：**

- `xgeXuiWidgetRemove`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiWidgetRemove

从父节点移除 widget。

**功能：**

你可以用它断开 widget 与父节点的关系，但不释放 widget 本身。

**函数原型：**

```c
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：要移除的 widget。

**返回值：**

无。

**资源归属：**

函数不释放 widget，调用者仍持有该节点。

**补充说明：**

移除后会清空 parent/prev/next sibling，并标记原父节点 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetRemove(w);
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiWidgetFree`

---

### xgeXuiWidgetSetId

设置 widget ID。

**功能：**

你可以用它给 widget 设置整数 ID，便于后续查找。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId);
```

**参数：**

- `pWidget`：widget 对象。
- `iId`：整数 ID。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

`pWidget` 为 `NULL` 时无效果。

**范例代码：**

```c
xgeXuiWidgetSetId(w, 1001);
```

**相关 API：**

- `xgeXuiWidgetGetId`
- `xgeXuiWidgetFindById`

---

### xgeXuiWidgetGetId

获取 widget ID。

**功能：**

你可以用它读取 widget 当前 ID。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetId(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 ID。
- `pWidget` 为 `NULL` 时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

ID 是否唯一由调用者维护。

**范例代码：**

```c
int id = xgeXuiWidgetGetId(w);
```

**相关 API：**

- `xgeXuiWidgetSetId`

---

### xgeXuiWidgetSetName

设置 widget 名称。

**功能：**

你可以用它给 widget 设置名称，便于调试或按名称查找。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName);
```

**参数：**

- `pWidget`：widget 对象。
- `sName`：名称字符串，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存借用指针，不复制字符串。

**补充说明：**

`sName == NULL` 时名称被设置为空字符串。调用者必须保证非空名称指针生命周期覆盖使用期。

**范例代码：**

```c
xgeXuiWidgetSetName(w, "main_panel");
```

**相关 API：**

- `xgeXuiWidgetGetName`
- `xgeXuiWidgetFindByName`

---

### xgeXuiWidgetGetName

获取 widget 名称。

**功能：**

你可以用它读取 widget 当前名称。

**函数原型：**

```c
XGE_API const char* xgeXuiWidgetGetName(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- 返回名称字符串。
- widget 无效或名称为空时返回空字符串。

**资源归属：**

返回值是借用指针，调用者不能释放。

**补充说明：**

名称指针由 `xgeXuiWidgetSetName` 的调用者负责保持有效。

**范例代码：**

```c
const char* name = xgeXuiWidgetGetName(w);
```

**相关 API：**

- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetFindById

按 ID 递归查找 widget。

**功能：**

你可以从 root 或任意子树开始查找第一个匹配 ID 的 widget。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId);
```

**参数：**

- `pRoot`：查找起点。
- `iId`：目标 ID。

**返回值：**

- 找到时返回 widget。
- 未找到或 `pRoot == NULL` 时返回 `NULL`。

**资源归属：**

返回值是借用指针，不转移所有权。

**补充说明：**

查找使用深度优先遍历。

**范例代码：**

```c
xge_xui_widget button = xgeXuiWidgetFindById(xgeXuiRoot(&ui), 1001);
```

**相关 API：**

- `xgeXuiWidgetSetId`

---

### xgeXuiWidgetFindByName

按名称递归查找 widget。

**功能：**

你可以从 root 或任意子树开始查找第一个匹配名称的 widget。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName);
```

**参数：**

- `pRoot`：查找起点。
- `sName`：目标名称，不能为 `NULL`。

**返回值：**

- 找到时返回 widget。
- 未找到或参数非法时返回 `NULL`。

**资源归属：**

返回值是借用指针。

**补充说明：**

名称比较使用 `strcmp`。

**范例代码：**

```c
xge_xui_widget panel = xgeXuiWidgetFindByName(xgeXuiRoot(&ui), "main_panel");
```

**相关 API：**

- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetSetRect

设置 widget 矩形。

**功能：**

你可以用它直接设置 widget 的绝对矩形，常用于 absolute layout 或 root 尺寸更新。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
```

**参数：**

- `pWidget`：widget 对象。
- `tRect`：目标矩形。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后会标记该 widget layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetRect(root, xgeRect(0.0f, 0.0f, 800.0f, 600.0f));
```

**相关 API：**

- `xgeXuiWidgetGetRect`
- `xgeXuiUpdate`

---

### xgeXuiWidgetGetRect

获取 widget 的 border 矩形。

**功能：**

你可以用它读取 widget 当前布局后的 `borderRect`。需要明确盒模型层级时，优先使用 `xgeXuiWidgetGetOuterRect`、`xgeXuiWidgetGetBorderRect`、`xgeXuiWidgetGetPaddingRect` 或 `xgeXuiWidgetGetContentRect`。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回其 `borderRect`。
- widget 无效时返回清零矩形。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局更新后读取更有意义。需要完整盒模型时，使用 `xgeXuiWidgetGetOuterRect`、`xgeXuiWidgetGetBorderRect`、`xgeXuiWidgetGetPaddingRect` 或 `xgeXuiWidgetGetContentRect`。

**范例代码：**

```c
xge_rect_t rect = xgeXuiWidgetGetRect(w);
```

**相关 API：**

- `xgeXuiWidgetSetRect`
- `xgeXuiWidgetGetOuterRect`
- `xgeXuiWidgetGetBorderRect`
- `xgeXuiWidgetGetPaddingRect`
- `xgeXuiWidgetGetContentRect`
- `xgeXuiUpdate`

---

### xgeXuiWidgetGetOuterRect

获取 widget 的 outer 矩形，包含 margin。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetOuterRect(xge_xui_widget pWidget);
```

无效 widget 返回清零矩形。

---

### xgeXuiWidgetGetBorderRect

获取 widget 的 border 矩形。当前等价于 `xgeXuiWidgetGetRect`。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetBorderRect(xge_xui_widget pWidget);
```

无效 widget 返回清零矩形。

---

### xgeXuiWidgetGetPaddingRect

获取 widget 的 padding 矩形。存在 border width 时，`paddingRect` 会从 `borderRect` 内扣除边框宽度。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetPaddingRect(xge_xui_widget pWidget);
```

无效 widget 返回清零矩形。

---

### xgeXuiWidgetGetContentRect

获取 widget 的 content 矩形，即扣除 padding 后用于子布局、内容裁剪和滚动 viewport 的区域。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetContentRect(xge_xui_widget pWidget);
```

无效 widget 返回清零矩形。

---

### xgeXuiWidgetSetStyle

设置 widget style。

**功能：**

你可以一次性替换 widget 的布局和外观样式。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle);
```

**参数：**

- `pWidget`：widget 对象。
- `pStyle`：style 输入，不能为 `NULL`。

**返回值：**

无。

**资源归属：**

函数复制 style 结构体，不接管输入指针。

**补充说明：**

grid columns 会被修正到至少 1，align 会被 clamp，负 radius 会被修正为 0。设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetStyle(w, &style);
```

**相关 API：**

- `xgeXuiWidgetGetStyle`
- `xgeXuiStyleDefault`

---

### xgeXuiWidgetGetStyle

获取 widget style。

**功能：**

你可以读取 widget 当前 style。

**函数原型：**

```c
XGE_API const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回内部 style 指针。
- widget 无效时返回 `NULL`。

**资源归属：**

返回值是借用指针，不能释放。

**补充说明：**

建议通过 setter 修改 style，避免绕过 dirty 标记。

**范例代码：**

```c
const xge_xui_style_t* style = xgeXuiWidgetGetStyle(w);
```

**相关 API：**

- `xgeXuiWidgetSetStyle`

---

### xgeXuiWidgetSetLayout

设置布局模式。

**功能：**

你可以设置 widget 子节点布局方式，例如 absolute、row、column、stack、grid 或 dock。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
```

**参数：**

- `pWidget`：widget 对象。
- `iLayout`：`XGE_XUI_LAYOUT_*`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout dirty。

**范例代码：**

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_DOCK);
```

**相关 API：**

- `xgeXuiWidgetSetGrid`
- `xgeXuiWidgetSetDock`
- `xgeXuiUpdate`

---

### xgeXuiWidgetSetDock

设置 widget 在父 DockLayout 中的停靠边。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetDock(xge_xui_widget pWidget, int iDock);
XGE_API int xgeXuiWidgetGetDock(xge_xui_widget pWidget);
```

**补充说明：**

`iDock` 支持 `XGE_XUI_DOCK_TOP/BOTTOM/LEFT/RIGHT/FILL`，`XGE_XUI_DOCK_CENTER` 是 `FILL` 的别名。父布局不是 `XGE_XUI_LAYOUT_DOCK` 时该字段不会影响布局。

---

### xgeXuiWidgetSetSize

设置 widget 宽高尺寸描述。

**功能：**

你可以设置 widget 的宽高解析规则。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：宽度描述。
- `tHeight`：高度描述。

**返回值：**

无。

**资源归属：**

函数复制尺寸结构体。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetSize(w, xgeXuiSizePercent(100.0f), xgeXuiSizeDip(40.0f));
```

**相关 API：**

- `xgeXuiSizePx`
- `xgeXuiSizeGrow`

---

### xgeXuiWidgetSetMinSize

设置最小尺寸。

**功能：**

你可以限制 widget 布局结果的最小宽高。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：最小宽度。
- `tHeight`：最小高度。

**返回值：**

无。

**资源归属：**

函数复制尺寸描述。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetMinSize(w, xgeXuiSizePx(80.0f), xgeXuiSizePx(24.0f));
```

**相关 API：**

- `xgeXuiWidgetSetMaxSize`
- `xgeXuiWidgetSetSize`

---

### xgeXuiWidgetSetMaxSize

设置最大尺寸。

**功能：**

你可以限制 widget 布局结果的最大宽高。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：最大宽度。
- `tHeight`：最大高度。

**返回值：**

无。

**资源归属：**

函数复制尺寸描述。

**补充说明：**

max 值解析为大于 0 时才作为上限。

**范例代码：**

```c
xgeXuiWidgetSetMaxSize(w, xgeXuiSizePx(320.0f), xgeXuiSizePx(0.0f));
```

**相关 API：**

- `xgeXuiWidgetSetMinSize`

---

### xgeXuiWidgetSetGrid

设置 grid 布局参数。

**功能：**

你可以配置 grid 列数、行高和行列间距。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap);
```

**参数：**

- `pWidget`：widget 对象。
- `iColumns`：列数，小于等于 0 时按 1。
- `fRowHeight`：行高，小于等于 0 时使用单元格宽度，形成方格网格。
- `fColumnGap`：列间距，小于等于 0 时按 0。
- `fRowGap`：行间距，小于等于 0 时按 0。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetGrid(panel, 3, 32.0f, 8.0f, 8.0f);
```

**相关 API：**

- `xgeXuiWidgetSetLayout`
- `xgeXuiWidgetSetGridColumnSpan`

---

### xgeXuiWidgetSetGridColumnSpan

设置 grid 子元素横跨列数。

**功能：**

你可以让 grid 布局中的某个子元素横跨多列，用于表单标题、宽按钮、卡片头部等场景。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetGridColumnSpan(xge_xui_widget pWidget, int iColumnSpan);
```

**参数：**

- `pWidget`：grid 子元素 widget。
- `iColumnSpan`：横跨列数，小于等于 1 时按 1；大于父 grid 列数时按父 grid 列数裁剪。

**返回值：**

无。

**补充说明：**

- 第一版只支持 column span，不支持 row span 或 CSS Grid 式占用矩阵。
- 如果当前行剩余列数不足，子元素会换到下一行再放置。

**范例代码：**

```c
xgeXuiWidgetSetGridColumnSpan(title, 2);
```

**相关 API：**

- `xgeXuiWidgetSetGrid`

---

### xgeXuiWidgetSetAlign

设置子项对齐方式。

**功能：**

你可以设置 widget 在父布局槽位中的水平和垂直对齐。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY);
```

**参数：**

- `pWidget`：widget 对象。
- `iAlignX`：水平对齐，使用 `XGE_XUI_ALIGN_*`。
- `iAlignY`：垂直对齐，使用 `XGE_XUI_ALIGN_*`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非法 align 会被 clamp 为 stretch。

**范例代码：**

```c
xgeXuiWidgetSetAlign(w, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_CENTER);
```

**相关 API：**

- `xgeXuiWidgetSetSize`

---

### xgeXuiWidgetSetZ

设置 widget z 顺序值。

**功能：**

你可以用它影响同级 widget 的绘制、hit test 和事件目标排序。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ);
```

**参数：**

- `pWidget`：widget 对象。
- `iZ`：z 值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后会标记父节点 paint dirty；没有父节点时标记自身。同级排序规则为 `layer > z > treeOrder`，`xgeXuiWidgetSetZ` 对应其中的 z 值。

**范例代码：**

```c
xgeXuiWidgetSetZ(overlayWidget, 100);
```

**相关 API：**

- `xgeXuiWidgetGetZ`
- `xgeXuiWidgetSetLayer`
- `xgeXuiWidgetGetTreeOrder`

---

### xgeXuiWidgetGetZ

获取 widget z 顺序值。

**功能：**

你可以读取 widget 当前 z 值。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 z 值。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

z 值用于 XUI 绘制排序、hit test 和事件目标选择；同级排序规则为 `layer > z > treeOrder`。

**范例代码：**

```c
int z = xgeXuiWidgetGetZ(overlayWidget);
```

**相关 API：**

- `xgeXuiWidgetSetZ`
- `xgeXuiWidgetGetLayer`
- `xgeXuiWidgetGetTreeOrder`

---

### xgeXuiWidgetSetLayer

设置 widget layer。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetLayer(xge_xui_widget pWidget, int iLayer);
```

`iLayer` 可取 `XGE_XUI_LAYER_NORMAL`、`XGE_XUI_LAYER_FLOATING`、`XGE_XUI_LAYER_POPUP`、`XGE_XUI_LAYER_MODAL`、`XGE_XUI_LAYER_TOOLTIP`、`XGE_XUI_LAYER_DRAG_ADORNER`、`XGE_XUI_LAYER_DEBUG`。非法值会回落到 `NORMAL`。

---

### xgeXuiWidgetGetLayer

获取 widget layer。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetLayer(xge_xui_widget pWidget);
```

无效 widget 返回 `XGE_XUI_LAYER_NORMAL`。

---

### xgeXuiWidgetGetTreeOrder

获取 widget 当前 treeOrder。

**函数原型：**

```c
XGE_API uint32_t xgeXuiWidgetGetTreeOrder(xge_xui_widget pWidget);
```

treeOrder 由 `xgeXuiWidgetAdd` 写入，用于同 layer、同 z 时保持稳定的树顺序；无效 widget 返回 0。

---

### xgeXuiWidgetSetOverflow

设置 widget overflow 策略。

**功能：**

`overflow` 是 widget 基础语义，取值为 `XGE_XUI_OVERFLOW_VISIBLE`、`XGE_XUI_OVERFLOW_CLIP`、`XGE_XUI_OVERFLOW_HIDDEN`、`XGE_XUI_OVERFLOW_SCROLL`。`clip`、`hidden`、`scroll` 都会启用当前 content rect 裁剪；`scroll` 只表示显式滚动视图类控件的滚动语义，不会让普通 widget 自动生成滚动条。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetOverflow(xge_xui_widget pWidget, int iOverflow);
```

**相关 API：**

- `xgeXuiWidgetGetOverflow`
- `xgeXuiWidgetSetClip`

---

### xgeXuiWidgetGetOverflow

获取 widget overflow 策略。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetOverflow(xge_xui_widget pWidget);
```

无效 widget 返回 `XGE_XUI_OVERFLOW_VISIBLE`。

---

### xgeXuiWidgetSetAnchorPx

设置像素锚点。

**功能：**

你可以用像素值配置 absolute/anchor 布局中的 left/top/right/bottom 约束。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `iAnchor`：`XGE_XUI_ANCHOR_*` 组合。
- `fLeft`：左锚点像素。
- `fTop`：上锚点像素。
- `fRight`：右锚点像素。
- `fBottom`：下锚点像素。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

anchor 标志会被过滤到合法位；四个值会转为 px size。

**范例代码：**

```c
xgeXuiWidgetSetAnchorPx(w, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 8.0f, 8.0f, 0.0f, 0.0f);
```

**相关 API：**

- `xgeXuiWidgetGetAnchor`

---

### xgeXuiWidgetGetAnchor

获取 anchor 标志。

**功能：**

你可以读取 widget 当前 anchor 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 anchor 标志。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数只返回标志，不返回具体边距值。

**范例代码：**

```c
int anchor = xgeXuiWidgetGetAnchor(w);
```

**相关 API：**

- `xgeXuiWidgetSetAnchorPx`

---

### xgeXuiWidgetSetMarginPx

设置像素 margin。

**功能：**

你可以设置 widget 四边外边距。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `fLeft` / `fTop` / `fRight` / `fBottom`：四边 margin 像素值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetMarginPx(w, 4.0f, 4.0f, 4.0f, 4.0f);
```

**相关 API：**

- `xgeXuiWidgetSetPaddingPx`

---

### xgeXuiWidgetSetPaddingPx

设置像素 padding。

**功能：**

你可以设置 widget 四边内边距，影响 content rect 和内容测量。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `fLeft` / `fTop` / `fRight` / `fBottom`：四边 padding 像素值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetPaddingPx(panel, 8.0f, 8.0f, 8.0f, 8.0f);
```

**相关 API：**

- `xgeXuiWidgetSetMarginPx`

---

### xgeXuiWidgetSetBackground

设置背景色。

**功能：**

你可以设置 widget 默认绘制背景颜色。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `iColor`：背景色。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后只标记 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetBackground(panel, XGE_COLOR_RGBA(32, 38, 46, 255));
```

**相关 API：**

- `xgeXuiWidgetSetRadius`
- `xgeXuiWidgetSetBorder`
- `xgeXuiWidgetSetFocusRing`

---

### xgeXuiWidgetSetRadius

设置圆角半径。

**功能：**

你可以设置 widget 背景绘制的圆角半径。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius);
```

**参数：**

- `pWidget`：widget 对象。
- `fRadius`：圆角半径，小于 0 时按 0。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 paint dirty。该接口是低层绘制回调；公开自绘优先使用 `xgeXuiWidgetSetOwnerDraw`，避免绕开 OwnerDraw mode 和 PaintInfo 约定。

**范例代码：**

```c
xgeXuiWidgetSetRadius(panel, 4.0f);
```

**相关 API：**

- `xgeXuiWidgetSetBackground`
- `xgeXuiWidgetSetBorder`

---

### xgeXuiWidgetSetBorder

设置 widget 基础边框。

**功能：**

你可以设置 widget 基类绘制的边框宽度和颜色。边框宽度会参与盒模型，`contentRect` 会在 `borderRect` 内继续扣除边框宽度和 padding。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetBorder(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `fWidth`：边框宽度，小于 0 时按 0。
- `iColor`：边框颜色；alpha 为 0 时不绘制。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后会标记 layout 和 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetBorder(panel, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
```

**相关 API：**

- `xgeXuiWidgetSetBackground`
- `xgeXuiWidgetSetFocusRing`
- `xgeXuiWidgetSetDisabledOverlay`
- `xgeXuiWidgetSetDebugOutline`

---

### xgeXuiWidgetSetFocusRing

设置 widget 基础焦点环。

**功能：**

你可以设置 widget 获得焦点时由基类自动绘制的焦点环。默认宽度为 0，不会影响已有控件的自绘焦点效果。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetFocusRing(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `fWidth`：焦点环宽度，小于 0 时按 0。
- `iColor`：焦点环颜色；alpha 为 0 时不绘制。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后只标记 paint dirty。只有当前 focused widget 会绘制焦点环。

**范例代码：**

```c
xgeXuiWidgetSetFocusRing(input, 1.5f, XGE_COLOR_RGBA(53, 174, 234, 255));
```

**相关 API：**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiSetFocus`
- `xgeXuiWidgetSetDisabledOverlay`

---

### xgeXuiWidgetSetDisabledOverlay

设置 widget 禁用态遮罩。

**功能：**

你可以设置 widget 禁用时由基类自动绘制的半透明遮罩。默认颜色透明，不改变现有控件视觉。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetDisabledOverlay(xge_xui_widget pWidget, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `iColor`：禁用遮罩颜色；alpha 为 0 时不绘制。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

遮罩在控件内容和 children 之后绘制，在 border/debug outline 之前绘制。

**范例代码：**

```c
xgeXuiWidgetSetDisabledOverlay(panel, XGE_COLOR_RGBA(237, 245, 250, 160));
```

**相关 API：**

- `xgeXuiWidgetSetEnabled`
- `xgeXuiWidgetSetDebugOutline`

---

### xgeXuiWidgetSetDebugOutline

设置 widget 调试描边。

**功能：**

你可以为单个 widget 设置始终绘制的调试描边，用于临时定位布局和裁剪问题。默认宽度为 0。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetDebugOutline(xge_xui_widget pWidget, float fWidth, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `fWidth`：调试描边宽度，小于 0 时按 0。
- `iColor`：调试描边颜色；alpha 为 0 时不绘制。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

调试描边在 background、内容、disabled overlay、border 和 focus ring 之后绘制。

**范例代码：**

```c
xgeXuiWidgetSetDebugOutline(panel, 1.0f, XGE_COLOR_RGBA(255, 80, 80, 220));
```

**相关 API：**

- `xgedbgXuiDebugOverlayPaint`
- `xgeXuiWidgetSetDisabledOverlay`

---

### xgeXuiWidgetGetFlags

获取 widget flags。

**功能：**

你可以读取 widget 当前可见、启用、可聚焦、裁剪、命中、输入穿透和 dirty 状态。

**函数原型：**

```c
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 flags。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

flags 使用 `XGE_XUI_WIDGET_*`。

**范例代码：**

```c
uint32_t flags = xgeXuiWidgetGetFlags(w);
```

**相关 API：**

- `xgeXuiWidgetSetVisible`
- `xgeXuiWidgetSetEnabled`
- `xgeXuiWidgetSetHitTestVisible`
- `xgeXuiWidgetSetInputTransparent`

---

### xgeXuiWidgetSetVisible

设置可见状态。

**功能：**

你可以显示或隐藏 widget。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
```

**参数：**

- `pWidget`：widget 对象。
- `bVisible`：非 0 可见，0 隐藏。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

状态变化会标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetVisible(overlayWidget, 1);
```

**相关 API：**

- `xgeXuiWidgetIsVisible`

---

### xgeXuiWidgetSetEnabled

设置启用状态。

**功能：**

你可以启用或禁用 widget。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
```

**参数：**

- `pWidget`：widget 对象。
- `bEnabled`：非 0 启用，0 禁用。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

禁用状态会影响命中、交互和可聚焦判断。

**范例代码：**

```c
xgeXuiWidgetSetEnabled(button, 0);
```

**相关 API：**

- `xgeXuiWidgetIsEnabled`

---

### xgeXuiWidgetSetHitTestVisible

设置 widget 是否参与 hit test。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetHitTestVisible(xge_xui_widget pWidget, int bVisible);
```

`bVisible` 为 0 时，该 widget 及其子树都不会成为命中目标。默认值为 1。

---

### xgeXuiWidgetIsHitTestVisible

读取 widget 是否参与 hit test。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsHitTestVisible(xge_xui_widget pWidget);
```

无效 widget 返回 0。

---

### xgeXuiWidgetSetInputTransparent

设置 widget 自身是否输入穿透。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetInputTransparent(xge_xui_widget pWidget, int bTransparent);
```

`bTransparent` 为非 0 时，该 widget 自身不会成为命中目标，但它的 children 仍可命中；如果没有 child 命中，事件会穿透到下层同级 widget。默认值为 0。

---

### xgeXuiWidgetIsInputTransparent

读取 widget 自身是否输入穿透。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsInputTransparent(xge_xui_widget pWidget);
```

无效 widget 返回 0。

---

### xgeXuiWidgetSetFocusable

设置可聚焦状态。

**功能：**

你可以允许或禁止 widget 获得焦点。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
```

**参数：**

- `pWidget`：widget 对象。
- `bFocusable`：非 0 可聚焦，0 不可聚焦。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

实际可聚焦还要求 widget 及其父链可见且启用。设置为可聚焦时会默认进入 TAB 顺序；如果只希望通过鼠标或代码聚焦，随后调用 `xgeXuiWidgetSetTabStop(widget, 0)`。

**范例代码：**

```c
xgeXuiWidgetSetFocusable(input, 1);
```

**相关 API：**

- `xgeXuiWidgetIsFocusable`
- `xgeXuiWidgetSetTabStop`
- `xgeXuiSetFocus`

---

### xgeXuiWidgetSetTabStop

设置 widget 是否进入 TAB 顺序。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetTabStop(xge_xui_widget pWidget, int bTabStop);
```

`tabStop` 只影响 TAB 遍历，不影响 `xgeXuiSetFocus` 手动聚焦。widget 仍必须满足可见、启用、focusable 才会被 TAB 选中。

TAB 遍历默认使用 page root 作为范围；存在可见 modal overlay 时，范围限制在最上层 modal 子树内。非 modal overlay 只有在当前焦点已位于该 overlay 子树时才限制 TAB 范围，避免 popup 打开后抢走根界面的 TAB 遍历。普通 widget 可通过 `xgeXuiWidgetSetFocusScope` 声明命名 focus scope，当前焦点位于该 scope 子树时，TAB 会限制在最近的可见启用 scope 内。

---

### xgeXuiWidgetIsTabStop

判断 widget 当前是否可通过 TAB 聚焦。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsTabStop(xge_xui_widget pWidget);
```

返回非 0 表示 widget 当前可进入 TAB 顺序；无效 widget 或不可聚焦 widget 返回 0。

---

### xgeXuiWidgetSetTabIndex

设置 TAB 遍历排序值。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetTabIndex(xge_xui_widget pWidget, int iTabIndex);
```

TAB 遍历按 `tabIndex` 升序，再按 `treeOrder` 升序。默认 `tabIndex` 为 0。

---

### xgeXuiWidgetGetTabIndex

读取 TAB 遍历排序值。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetTabIndex(xge_xui_widget pWidget);
```

无效 widget 返回 0。

---

### xgeXuiWidgetSetFocusScope

设置 widget 是否作为命名 focus scope。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetFocusScope(xge_xui_widget pWidget, int bFocusScope);
```

focus scope 只限制键盘焦点遍历和 default/cancel action 的查找范围，不影响鼠标命中、事件路由和 `xgeXuiSetFocus` 手动聚焦。当前焦点位于某个 focus scope 子树内时，FocusManager 使用最近的可见启用 focus scope；没有命中命名 scope 时回退到 page root。可见 modal overlay 和焦点所在的非 modal overlay 优先级高于命名 focus scope。

---

### xgeXuiWidgetIsFocusScope

判断 widget 是否声明为命名 focus scope。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsFocusScope(xge_xui_widget pWidget);
```

返回非 0 表示 widget 已声明为 focus scope。该函数只反映声明开关；scope 是否在当前帧生效，还取决于当前焦点位置、父链可见启用状态以及 overlay/modal scope 优先级。

---

### xgeXuiWidgetSetDefaultAction

设置 widget 的 Enter 默认动作。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetDefaultAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
```

`procAction` 为 NULL 时清空默认动作。按下 Enter 且常规事件路由无人消费、没有 pointer capture 时，FocusManager 会在当前 focus scope 内按 TAB 顺序查找第一个可见启用的 default action 并调用它。可见 modal overlay 会限制 action 查找范围；非 modal overlay 只有在当前焦点位于其子树内时才限制范围；普通命名 focus scope 在 overlay/modal scope 之后生效。

---

### xgeXuiWidgetSetCancelAction

设置 widget 的 Escape 取消动作。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetCancelAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser);
```

`procAction` 为 NULL 时清空取消动作。按下 Escape 且常规事件路由无人消费、没有 pointer capture 时，FocusManager 会在当前 focus scope 内按 TAB 顺序查找第一个可见启用的 cancel action 并调用它。pointer capture 存在时 Escape 优先释放 capture，并依次发送 `XGE_EVENT_XUI_CAPTURE_LOST` 与 `XGE_EVENT_XUI_CAPTURE_CANCEL`。普通命名 focus scope 在 overlay/modal scope 之后生效。

---

### xgeXuiWidgetSetImeMode

设置 widget 的 IME 申请策略。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetImeMode(xge_xui_widget pWidget, int iImeMode);
```

可用值：`XGE_XUI_IME_DISABLED`、`XGE_XUI_IME_ENABLED`、`XGE_XUI_IME_AUTO`。默认 disabled。焦点进入 widget 时，XUI context 会统一调用 `xgeImeSetEnabled` 同步系统 IME，并刷新当前焦点 widget 的候选框参考区域；焦点离开或目标不申请 IME 时会禁用 IME 并清空候选框区域。

---

### xgeXuiWidgetGetImeMode

读取 widget 的 IME 申请策略。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetImeMode(xge_xui_widget pWidget);
```

无效 widget 返回 `XGE_XUI_IME_DISABLED`。`Input` 和 `TextEdit` 默认 enabled；password input 会切到 disabled，除非调用方显式覆盖。

---

### xgeXuiWidgetSetImeCandidateRect

为 widget 注册 IME 候选框参考区域解析器。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetImeCandidateRect(xge_xui_widget pWidget, xge_xui_ime_candidate_rect_proc procRect, void* pUser);
```

`procRect` 返回系统 IME 候选框应锚定的矩形。传入 `NULL` 会清除自定义解析器；未注册解析器的 widget 使用 `contentRect` 作为默认区域，必要时退回到 `borderRect` / `rect`。如果 widget 当前拥有焦点，设置会立即刷新 context 内缓存。

---

### xgeXuiWidgetGetImeCandidateRect

读取指定 widget 的候选框参考区域。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetImeCandidateRect(xge_xui_widget pWidget);
```

该函数只解析 widget 的区域，不判断它是否正在申请 IME。需要平台后端当前焦点区域时，应使用 `xgeXuiGetImeCandidateRect`。

---

### xgeXuiHasImeCandidateRect / xgeXuiGetImeCandidateRect

读取当前 context 焦点 widget 的 IME 候选框参考区域。

**函数原型：**

```c
XGE_API int xgeXuiHasImeCandidateRect(xge_xui_context pContext);
XGE_API xge_rect_t xgeXuiGetImeCandidateRect(xge_xui_context pContext);
```

只有当前焦点 widget 申请 IME 时，`xgeXuiHasImeCandidateRect` 才返回 1。`xgeXuiGetImeCandidateRect` 会按需刷新缓存；无有效区域时返回全 0 矩形。`Input` 和 `TextEdit` 初始化时会自动注册自己的光标位置解析器。

---

### xgeXuiWidgetSetClip

设置裁剪状态。

**功能：**

这是便捷 API，等价于在 `XGE_XUI_OVERFLOW_VISIBLE` 和 `XGE_XUI_OVERFLOW_CLIP` 之间切换。需要表达完整 overflow 语义时使用 `xgeXuiWidgetSetOverflow`。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
```

**参数：**

- `pWidget`：widget 对象。
- `bClip`：非 0 启用裁剪，0 关闭裁剪。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

- 状态变化会触发布局和绘制刷新。
- `bClip` 为 0 时，子节点可以按布局结果溢出显示；`overflow: visible` 也允许溢出子节点在父边界外被命中。
- `bClip` 非 0 时，绘制裁剪到 `tContentRect`；命中点如果仍在 widget 矩形内但落在 `tContentRect` 外，会命中该 widget 本身，不会命中溢出的子节点。

**范例代码：**

```c
xgeXuiWidgetSetClip(scroll, 1);
```

**相关 API：**

- `xgeXuiPaint`

---

### xgeXuiWidgetSetCaptureEvent

设置捕获阶段事件回调。

**功能：**

你可以用它在事件冒泡前处理事件。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent);
```

**参数：**

- `pWidget`：widget 对象。
- `procEvent`：捕获事件回调，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存函数指针，不接管用户数据。

**补充说明：**

捕获回调返回 `XGE_XUI_EVENT_HANDLED` 时会继续后续普通派发，但最终结果为 handled；返回 `XGE_XUI_EVENT_CONSUMED` 时会阻止后续普通派发。

**范例代码：**

```c
xgeXuiWidgetSetCaptureEvent(w, on_capture);
```

**相关 API：**

- `xgeXuiDispatchEvent`

---

### xgeXuiWidgetSetMeasure

设置测量回调。

**功能：**

你可以用它为自定义 widget 提供内容尺寸。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure);
```

**参数：**

- `pWidget`：widget 对象。
- `procMeasure`：测量回调，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存回调指针。

**补充说明：**

设置后标记 layout dirty。测量时会把 widget 的 `pUser` 传给回调。

**范例代码：**

```c
xgeXuiWidgetSetMeasure(label_widget, measure_label);
```

**相关 API：**

- `xgeXuiSizeContent`
- `xgeXuiUpdate`

---

### xgeXuiWidgetSetPaintBefore

设置基础背景之前的绘制回调。

**功能：**

用于自定义 underlay 绘制，例如 backdrop。该回调在 widget 基础背景、content clip 和普通 `procPaint` 之前执行。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaintBefore(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**参数：**

- `pWidget`：widget 对象。
- `procPaint`：绘制回调，可以为 `NULL`。
- `pUser`：用户数据指针。

**返回值：**

无。

**资源归属：**

函数只保存回调和 `pUser` 借用指针。

**补充说明：**

设置后标记 paint dirty。回调受父级 clip 约束，但不受当前 widget 自身 content clip 截断。

---

### xgeXuiWidgetSetPaint

设置绘制回调。

**功能：**

你可以用它为自定义 widget 提供绘制逻辑和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**参数：**

- `pWidget`：widget 对象。
- `procPaint`：绘制回调，可以为 `NULL`。
- `pUser`：用户数据指针。

**返回值：**

无。

**资源归属：**

函数只保存回调和 `pUser` 借用指针。

**补充说明：**

设置后标记 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetPaint(w, paint_custom, user);
```

**相关 API：**

- `xgeXuiPaint`
- `xgeXuiWidgetSetOwnerDraw`

---

### xgeXuiWidgetSetPaintAfter

设置所有子树之后的装饰绘制回调。

**功能：**

用于追加 overlay、装饰线、调试辅助等绘制，不替换 widget 默认绘制。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaintAfter(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**参数：**

- `pWidget`：widget 对象。
- `procPaint`：绘制回调，可以为 `NULL`。
- `pUser`：用户数据指针。

**补充说明：**

设置后标记 paint dirty。`XGE_XUI_OWNER_DRAW_FULL` 会跳过 `paintAfter`；`CONTENT` 和 `CONTENT_AND_CHILDREN` 仍保留当前 widget 的 `paintAfter`。

---

### xgeXuiWidgetSetOwnerDraw

设置 Widget 级 OwnerDraw 自绘回调。

**功能：**

按 mode 替换 widget 内容、内容加子树，或完整替换 widget 绘制链。回调接收 `xge_xui_paint_info_t`，可读取盒模型矩形、状态、样式、DIP scale 和 control 指针。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetOwnerDraw(xge_xui_widget pWidget, int iMode, xge_xui_owner_draw_proc procDraw, void* pUser);
XGE_API void xgeXuiWidgetSetOwnerDrawControl(xge_xui_widget pWidget, void* pControl);
XGE_API int xgeXuiWidgetGetOwnerDrawMode(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。
- `iMode`：`XGE_XUI_OWNER_DRAW_CONTENT`、`XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN` 或 `XGE_XUI_OWNER_DRAW_FULL`。
- `procDraw`：自绘回调；传 `NULL` 会关闭 OwnerDraw。
- `pUser`：用户数据指针。
- `pControl`：可选控件对象指针，会写入 `xge_xui_paint_info_t::pControl`。

**补充说明：**

`CONTENT` 保留 children 和基础装饰；`CONTENT_AND_CHILDREN` 跳过 children；`FULL` 跳过 Widget 基础绘制链。跳过子树绘制时，基础层会清理子树 paint dirty，避免后续刷新冒泡失效。

**相关 API：**

- `xgeXuiWidgetSetPaintBefore`
- `xgeXuiWidgetSetPaintAfter`
- `xgeXuiWidgetMarkPaint`

---

### xgeXuiWidgetIsVisible

判断 widget 是否可见。

**功能：**

你可以用它读取 visible 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

可见返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`pWidget == NULL` 时返回 0。

**范例代码：**

```c
if (xgeXuiWidgetIsVisible(w)) {
}
```

**相关 API：**

- `xgeXuiWidgetSetVisible`

---

### xgeXuiWidgetIsEnabled

判断 widget 是否启用。

**功能：**

你可以用它读取 enabled 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

启用返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`pWidget == NULL` 时返回 0。

**范例代码：**

```c
int enabled = xgeXuiWidgetIsEnabled(w);
```

**相关 API：**

- `xgeXuiWidgetSetEnabled`

---

### xgeXuiWidgetIsFocusable

判断 widget 是否可聚焦。

**功能：**

你可以用它判断 widget 是否满足可见、启用、focusable 三个条件。父链不可见或禁用时也会返回 0。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

可聚焦返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数用于 `xgeXuiSetFocus` 的合法性判断。

**范例代码：**

```c
if (xgeXuiWidgetIsFocusable(input)) {
    xgeXuiSetFocus(&ui, input);
}
```

**相关 API：**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiWidgetSetTabStop`
- `xgeXuiSetFocus`

---

### xgeXuiWidgetMarkLayout

标记 layout dirty。

**功能：**

你可以用它通知 XUI 某个 widget 的布局需要重新计算。

**函数原型：**

```c
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非批处理状态下会向父链传播 dirty 并请求刷新；批处理状态下只记录批处理 dirty。

**范例代码：**

```c
xgeXuiWidgetMarkLayout(w);
```

**相关 API：**

- `xgeXuiLayoutBatchBegin`
- `xgeXuiUpdate`

---

### xgeXuiWidgetMarkPaint

标记 paint dirty。

**功能：**

你可以用它通知 XUI 某个 widget 需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非批处理状态下会向父链传播 dirty、尝试 invalidate widget 矩形并请求刷新。

**范例代码：**

```c
xgeXuiWidgetMarkPaint(w);
```

**相关 API：**

- `xgeXuiPaint`
- `xgeXuiRefreshRequest`

---

### xgeXuiWidgetMarkStyle

标记 style cache dirty。

```c
XGE_API void xgeXuiWidgetMarkStyle(xge_xui_widget pWidget);
```

第一版 XSON loader 会把 layout/visual 高频字段解析到 widget 的 `xge_xui_style_t`，这个结构就是轻量 style cache。`xgeXuiWidgetMarkStyle` 设置 `XGE_XUI_WIDGET_DIRTY_STYLE`，递增 widget style version，并同时触发布局与绘制 dirty。

---

### xgeXuiHitTest

命中测试。

**功能：**

你可以用它根据坐标找到当前最合适的 widget 目标。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
```

**参数：**

- `pContext`：XUI context。
- `fX`：X 坐标。
- `fY`：Y 坐标。

**返回值：**

- 命中时返回 widget。
- context 无效或无命中时返回 `NULL`。

**资源归属：**

返回值是借用指针。

**补充说明：**

事件派发中有坐标的事件会使用该函数寻找目标。

**范例代码：**

```c
xge_xui_widget hit = xgeXuiHitTest(&ui, x, y);
```

**相关 API：**

- `xgeXuiDispatchEvent`

---

### xgeXuiSetFocus

设置焦点 widget。

**功能：**

你可以用它改变当前键盘/文本事件焦点。

**函数原型：**

```c
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pContext`：XUI context。
- `pWidget`：目标 widget，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数不接管 widget 所有权。

**补充说明：**

不可聚焦 widget 会被忽略，跨 context 的 widget 也会被忽略。`tabStop` 不影响手动设置焦点；它只影响 TAB 遍历。焦点变化会派发 focus out/in 事件并标记旧/新焦点重绘。

**范例代码：**

```c
xgeXuiSetFocus(&ui, input);
```

**相关 API：**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiWidgetIsFocusable`
- `xgeXuiWidgetSetTabStop`

---

### xgeXuiSetCapture

设置 pointer 0 的事件捕获 widget。

**功能：**

你可以用它让鼠标/legacy pointer 事件优先派发给指定 widget，常用于拖动和按下捕获。需要多触点或自定义 pointer id 时使用 `xgeXuiSetPointerCapture`。

**函数原型：**

```c
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pContext`：XUI context。
- `pWidget`：捕获 widget；传 `NULL` 释放捕获。

**返回值：**

无。

**资源归属：**

函数只保存借用指针。

**补充说明：**

context 无效时无效果。`pWidget` 必须属于当前 context，且处于可见、启用状态；否则调用不会改变当前 capture。传 `NULL` 属于主动释放，不发送 `XGE_EVENT_XUI_CAPTURE_LOST` 或 `XGE_EVENT_XUI_CAPTURE_CANCEL`。当 capture 被其它 widget 替换，或 capture widget 被隐藏、禁用、移除、销毁、Escape 取消时，XUI 会自动释放 capture，并向旧 capture widget 依次发送 `XGE_EVENT_XUI_CAPTURE_LOST` 与 `XGE_EVENT_XUI_CAPTURE_CANCEL`。`xgeXuiSetCapture` 等价于 `xgeXuiSetPointerCapture(pContext, 0, pWidget)`。

**范例代码：**

```c
xgeXuiSetCapture(&ui, slider);
```

**相关 API：**

- `xgeXuiDispatchEvent`
- `xgeXuiSetPointerCapture`
- `xgeXuiReleaseWidgetCapture`

---

### xgeXuiSetPointerCapture

设置指定 pointer id 的事件捕获 widget。

**函数原型：**

```c
XGE_API void xgeXuiSetPointerCapture(xge_xui_context pContext, uint64_t iPointerId, xge_xui_widget pWidget);
```

`iPointerId` 为 `0` 时同步 legacy `pCapture`；非 0 pointer id 保存在 context 的 capture 表中。主动传 `NULL` 释放指定 pointer，不发送 lost/cancel；替换、隐藏、禁用、移除、销毁和 Escape 会依次发送 `XGE_EVENT_XUI_CAPTURE_LOST` 与 `XGE_EVENT_XUI_CAPTURE_CANCEL`，事件副本中的 `iPointerId` 表示丢失 capture 的 pointer。

---

### xgeXuiGetPointerCapture

读取指定 pointer id 当前捕获的 widget。

```c
XGE_API xge_xui_widget xgeXuiGetPointerCapture(xge_xui_context pContext, uint64_t iPointerId);
```

无捕获或 context 无效时返回 `NULL`。

---

### xgeXuiHasCapture

判断 context 中是否存在任意 pointer capture。

```c
XGE_API int xgeXuiHasCapture(xge_xui_context pContext);
```

---

### xgeXuiWidgetHasCapture

判断指定 widget 是否持有任意 pointer capture。

```c
XGE_API int xgeXuiWidgetHasCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

---

### xgeXuiReleaseWidgetCapture

主动释放指定 widget 当前持有的全部 pointer capture。

```c
XGE_API void xgeXuiReleaseWidgetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

该接口面向控件 `Unit`、销毁前清理和显式取消拖拽收尾：它会释放 `pWidget` 持有的 pointer 0 与非 0 pointer capture，且不发送 `XGE_EVENT_XUI_CAPTURE_LOST` 或 `XGE_EVENT_XUI_CAPTURE_CANCEL`。替换 capture、隐藏、禁用、移除、销毁和 Escape 仍由核心路径发送 lost/cancel 通知。

---

### xgeXuiDispatchEvent

派发单个事件。

**功能：**

你可以用它把 XGE 输入事件派发到 XUI widget tree。

**函数原型：**

```c
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：事件指针，不能为 `NULL` 才能生效。

**返回值：**

- 全路由无人处理时返回 `XGE_XUI_EVENT_CONTINUE`。
- 至少一个回调返回 handled 且无人 consumed 时返回 `XGE_XUI_EVENT_HANDLED`。
- 任一回调返回 consumed 或内部策略消费事件时返回 `XGE_XUI_EVENT_CONSUMED`。

**资源归属：**

函数不保存事件指针。

**补充说明：**

坐标事件会先按 `pEvent->iPointerId` 查找 pointer capture；命中时优先给 capture，否则走 hit test；其他事件走 legacy pointer 0 capture 或 focus，focus 为空时走 root。分发时会先按 root-to-target 调用 `procCaptureEvent`，再调用 target 的 `procEvent`，最后沿 target-to-root 冒泡到父级 `procEvent`。`handled` 会继续当前路由但阻止 XGE fallback 和 default/cancel action；`consumed` 会立即停止当前路由。回调收到的事件副本会填充 `iXuiPhase`、`pXuiOriginalTarget`、`pXuiCurrentTarget`、`pXuiCapture` 和 `bXuiCaptured`，调用方传入的原始事件不会被修改。

**范例代码：**

```c
xgeXuiDispatchEvent(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiDispatchQueuedEvents`

---

### xgeXuiEventPush

压入事件队列。

**功能：**

你可以把事件复制到 XUI 队列，稍后统一派发。

**函数原型：**

```c
XGE_API int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：事件指针，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 队列已满返回 `XGE_ERROR`。

**资源归属：**

函数复制事件结构体，不持有 `pEvent` 指针。

**补充说明：**

队列容量为 `XGE_XUI_EVENT_QUEUE_CAPACITY`。

**范例代码：**

```c
xgeXuiEventPush(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPop`
- `xgeXuiDispatchQueuedEvents`

---

### xgeXuiEventPop

弹出事件队列。

**功能：**

你可以从 XUI 队列取出最早进入的事件。

**函数原型：**

```c
XGE_API int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：输出事件，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 队列为空返回 `XGE_ERROR`。

**资源归属：**

函数写入调用者提供的事件结构体。

**补充说明：**

队列使用环形 head/tail。

**范例代码：**

```c
xge_event_t event;
xgeXuiEventPop(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiEventCount`

---

### xgeXuiEventCount

获取事件队列长度。

**功能：**

你可以用它查询当前排队事件数量。

**函数原型：**

```c
XGE_API int xgeXuiEventCount(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效时返回事件数量。
- context 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该数量不会超过 `XGE_XUI_EVENT_QUEUE_CAPACITY`。

**范例代码：**

```c
int count = xgeXuiEventCount(&ui);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiEventPop`

---

### xgeXuiDispatchQueuedEvents

派发当前队列中的事件。

**功能：**

你可以用它一次处理当前已经排队的事件。

**函数原型：**

```c
XGE_API int xgeXuiDispatchQueuedEvents(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 成功返回已处理事件数量。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

函数只处理调用开始时的队列长度；派发过程中新增事件会留到后续处理。

**范例代码：**

```c
int processed = xgeXuiDispatchQueuedEvents(&ui);
```

**相关 API：**

- `xgeXuiDispatchEvent`
- `xgeXuiEventPush`

---

### xgeXuiUpdate

更新 XUI 布局。

**功能：**

你可以在绘制前调用它，让 XUI 根据 root rect、style 和 measure 回调计算布局。

**函数原型：**

```c
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
```

**参数：**

- `pContext`：XUI context。
- `fDelta`：帧间隔。当前实现暂未使用。

**返回值：**

- 成功返回 `XGE_OK`。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

root 宽高小于等于 0 时会使用当前 XGE 窗口宽高。更新后清空 dirty layout 计数。

**范例代码：**

```c
xgeXuiUpdate(&ui, dt);
```

**相关 API：**

- `xgeXuiPaint`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiPaint

绘制 XUI。

**功能：**

你可以用它遍历 widget tree，调用 host 绘制背景、裁剪、控件内容和自定义 paint。

**函数原型：**

```c
XGE_API int xgeXuiPaint(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 成功绘制时返回 paint command 数量。
- 无需重绘时返回 0。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果 root 没有 paint dirty，会清空 paint/refresh 状态并返回 0。绘制后会恢复 active host/context。

**范例代码：**

```c
xgeXuiUpdate(&ui, dt);
xgeXuiPaint(&ui);
```

**相关 API：**

- `xgeXuiUpdate`
- `xgeXuiWidgetMarkPaint`

---

## API 参考：Text

### xgeXuiTextInit

初始化 XUI 文本缓冲。

**功能：**

为 `xge_xui_text_t` 分配初始 UTF-8 文本缓冲和 IME composition 缓冲，作为输入框、文本编辑控件或自定义文本状态的基础结构。

**函数原型：**

```c
XGE_API int xgeXuiTextInit(xge_xui_text pText);
```

**参数：**

- `pText`：要初始化的文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存分配失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会通过 xrt allocator 为文本对象内部缓冲分配内存。调用者必须在不再使用时调用 `xgeXuiTextUnit` 释放。

**补充说明：**

- 初始化后 `sText` 和 `sComposition` 都是空字符串。
- `xge_xui_text_t` 是透明结构，但不要直接释放内部指针。

**范例代码：**

```c
xge_xui_text_t text;
if (xgeXuiTextInit(&text) == XGE_OK) {
	xgeXuiTextSet(&text, "hello");
	xgeXuiTextUnit(&text);
}
```

**相关 API：**

- `xgeXuiTextUnit`
- `xgeXuiTextSet`

---

### xgeXuiTextUnit

释放 XUI 文本缓冲。

**功能：**

释放 `xge_xui_text_t` 内部持有的主文本缓冲和 IME composition 缓冲，并将结构清零。

**函数原型：**

```c
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
```

**参数：**

- `pText`：要释放的文本对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

只释放 `xgeXuiTextInit` 或后续扩容创建的内部缓冲，不释放 `pText` 结构本身。

**补充说明：**

- 释放后文本对象回到全零状态。
- 如果文本对象嵌在控件结构内，由控件的 `Unit` 函数负责调用。

**范例代码：**

```c
xge_xui_text_t text;
xgeXuiTextInit(&text);
xgeXuiTextUnit(&text);
```

**相关 API：**

- `xgeXuiTextInit`

---

### xgeXuiTextSet

设置完整文本内容。

**功能：**

用新的 UTF-8 字符串替换文本缓冲内容，并把游标和选择区移动到文本末尾。

**函数原型：**

```c
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：新的 UTF-8 文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 扩容失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会复制 `sText` 内容，调用者可以在返回后释放或修改原始字符串。

**补充说明：**

- 文本长度以 UTF-8 字节数保存。
- 设置主文本会清空 composition。

**范例代码：**

```c
xgeXuiTextSet(&text, "player name");
```

**相关 API：**

- `xgeXuiTextInsert`
- `xgeXuiTextClearComposition`

---

### xgeXuiTextInsert

在游标位置插入文本。

**功能：**

将 UTF-8 字符串插入当前游标位置。如果存在选择区，会先删除选择区，再插入新文本。

**函数原型：**

```c
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：要插入的 UTF-8 字符串，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 初始化或扩容失败返回对应错误码。

**资源归属：**

函数复制插入内容，不持有 `sText` 指针。

**补充说明：**

- 如果文本对象尚未初始化，当前实现会尝试自动初始化。
- 插入后游标移动到插入文本之后，选择区折叠到游标处。

**范例代码：**

```c
xgeXuiTextSetCursor(&text, 0);
xgeXuiTextInsert(&text, "XGE ");
```

**相关 API：**

- `xgeXuiTextSetCursor`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextInsertCodepoint

插入一个 Unicode codepoint。

**功能：**

将 `uint32_t` codepoint 编码为 UTF-8，并插入当前游标位置。

**函数原型：**

```c
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
```

**参数：**

- `pText`：文本对象。
- `iCodepoint`：要插入的 Unicode codepoint。

**返回值：**

- 成功返回 `XGE_OK`。
- 不支持的 codepoint 返回 `XGE_ERROR_UNSUPPORTED`。
- 插入失败返回 `xgeXuiTextInsert` 的错误码。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当前实现支持 1 到 3 字节 UTF-8 编码范围，并排除 surrogate 区间。
- 超出当前支持范围的字符会返回不支持。

**范例代码：**

```c
xgeXuiTextInsertCodepoint(&text, 0x4E2D);
```

**相关 API：**

- `xgeXuiTextInsert`
- `xgeXuiTextInputEvent`

---

### xgeXuiTextDeleteBack

删除游标前一个字符。

**功能：**

实现退格行为。如果存在选择区，则删除选择区；否则删除游标前一个 UTF-8 字符。

**函数原型：**

```c
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空或文本未初始化返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不释放文本对象，只在内部缓冲中移动内容。

**补充说明：**

- 游标位于开头时返回成功但不修改内容。
- 删除后会清空 composition。

**范例代码：**

```c
xgeXuiTextDeleteBack(&text);
```

**相关 API：**

- `xgeXuiTextDeleteForward`
- `xgeXuiTextSetSelection`

---

### xgeXuiTextDeleteForward

删除游标后一个字符。

**功能：**

实现 Delete 键行为。如果存在选择区，则删除选择区；否则删除游标后的一个 UTF-8 字符。

**函数原型：**

```c
XGE_API int xgeXuiTextDeleteForward(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空或文本未初始化返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不释放文本对象，只在内部缓冲中移动内容。

**补充说明：**

- 游标位于末尾时返回成功但不修改内容。
- 删除后会清空 composition。

**范例代码：**

```c
xgeXuiTextDeleteForward(&text);
```

**相关 API：**

- `xgeXuiTextDeleteBack`
- `xgeXuiTextGetCursor`

---

### xgeXuiTextSetCursor

设置文本游标位置。

**功能：**

把游标移动到指定 UTF-8 字节偏移，并自动夹到合法字符边界，同时清空选择区。

**函数原型：**

```c
XGE_API void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `iCursor`：目标字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 位置小于 0 会夹到 0。
- 位置大于文本长度会夹到文本末尾。
- 如果落在 UTF-8 continuation byte 上，会向前移动到字符起点。

**范例代码：**

```c
xgeXuiTextSetCursor(&text, text.iSize);
```

**相关 API：**

- `xgeXuiTextGetCursor`
- `xgeXuiTextSetSelection`

---

### xgeXuiTextGetCursor

获取文本游标位置。

**功能：**

返回当前游标的 UTF-8 字节偏移，并在返回前把游标修正到合法字符边界。

**函数原型：**

```c
XGE_API int xgeXuiTextGetCursor(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 返回当前游标字节偏移。
- `pText` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值是字节偏移，不是 Unicode 字符序号。

**范例代码：**

```c
int cursor = xgeXuiTextGetCursor(&text);
```

**相关 API：**

- `xgeXuiTextSetCursor`

---

### xgeXuiTextSetSelection

设置文本选择区。

**功能：**

设置选择区起点和终点，并把游标移动到选择区终点。

**函数原型：**

```c
XGE_API void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `iStart`：选择起点字节偏移。
- `iEnd`：选择终点字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 起点和终点都会被夹到合法 UTF-8 字符边界。
- 可以传入反向选择，读取时会自动归一化为小值到大值。

**范例代码：**

```c
xgeXuiTextSetSelection(&text, 0, text.iSize);
```

**相关 API：**

- `xgeXuiTextGetSelection`
- `xgeXuiTextInsert`

---

### xgeXuiTextGetSelection

获取文本选择区。

**功能：**

读取当前选择区，并将选择区归一化为 `[start, end]` 顺序。

**函数原型：**

```c
XGE_API void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `pStart`：用于接收选择起点，可为 `NULL`。
- `pEnd`：用于接收选择终点，可为 `NULL`。

**返回值：**

无。

**资源归属：**

输出参数由调用者持有。

**补充说明：**

- `pText` 为 `NULL` 时起点和终点按 0 返回。
- 输出值是 UTF-8 字节偏移。

**范例代码：**

```c
int start;
int end;
xgeXuiTextGetSelection(&text, &start, &end);
```

**相关 API：**

- `xgeXuiTextSetSelection`

---

### xgeXuiTextSetComposition

设置 IME 组合文本。

**功能：**

保存系统 IME 正在组合但尚未提交的 UTF-8 文本，供输入框绘制候选状态或预编辑文本。

**函数原型：**

```c
XGE_API int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：composition 文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 扩容失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数复制 `sText` 内容，不持有外部字符串指针。

**补充说明：**

- composition 不会立刻写入主文本缓冲。
- 系统提交文本后应通过 `XGE_EVENT_TEXT` 或直接调用插入 API 写入主文本。

**范例代码：**

```c
xgeXuiTextSetComposition(&text, "zhong");
```

**相关 API：**

- `xgeXuiTextClearComposition`
- `xgeXuiTextGetComposition`

---

### xgeXuiTextClearComposition

清空 IME 组合文本。

**功能：**

将 composition 缓冲重置为空字符串。

**函数原型：**

```c
XGE_API void xgeXuiTextClearComposition(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 composition 缓冲，只清空内容。

**补充说明：**

- 设置主文本、插入文本、删除文本时会自动清空 composition。

**范例代码：**

```c
xgeXuiTextClearComposition(&text);
```

**相关 API：**

- `xgeXuiTextSetComposition`
- `xgeXuiTextGetComposition`

---

### xgeXuiTextGetComposition

获取 IME 组合文本。

**功能：**

返回当前 composition 字符串，用于输入框绘制预编辑文本。

**函数原型：**

```c
XGE_API const char* xgeXuiTextGetComposition(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 返回 composition 字符串。
- 文本对象为空或未初始化时返回空字符串。

**资源归属：**

返回值是借用指针，由文本对象内部持有。调用者不能释放，也不应长期保存。

**补充说明：**

- 后续 `xgeXuiTextSetComposition`、`xgeXuiTextClearComposition` 或 `xgeXuiTextUnit` 可能让该指针失效或内容变化。

**范例代码：**

```c
const char* composing = xgeXuiTextGetComposition(&text);
```

**相关 API：**

- `xgeXuiTextSetComposition`
- `xgeXuiTextClearComposition`

---

### xgeXuiTextInputEvent

处理文本输入事件。

**功能：**

把 XGE 文本事件和 IME 事件应用到 `xge_xui_text_t`：文本事件插入 codepoint，IME start/update/end 维护 composition 状态。

**函数原型：**

```c
XGE_API int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent);
```

**参数：**

- `pText`：文本对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件或插入失败返回 `XGE_XUI_EVENT_CONTINUE`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不接管事件对象。事件数据由事件生产方管理。

**补充说明：**

- `XGE_EVENT_TEXT` 使用 `pEvent->iCodepoint`。
- `XGE_EVENT_IME_UPDATE` 会读取 `pEvent->pData` 中的 `xge_ime_event_t`。
- 该函数只处理文本缓冲，不负责标记控件重绘；控件事件过程应在内容变化后标记 paint。

**范例代码：**

```c
int handled = xgeXuiTextInputEvent(&text, event);
if (handled == XGE_XUI_EVENT_CONSUMED) {
	xgeXuiWidgetMarkPaint(widget);
}
```

**相关 API：**

- `xgeXuiInputEvent`
- `xgeXuiTextInsertCodepoint`

---

## API 参考：Button

### xgeXuiButtonInit

初始化按钮控件。

**功能：**

把 `xge_xui_button_t` 绑定到指定 widget，并安装按钮默认事件处理、绘制回调、主题颜色和可聚焦状态。

**函数原型：**

```c
XGE_API int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pButton`：按钮对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载按钮的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

按钮对象和 widget 都由调用者持有。函数不会分配按钮对象，也不会接管 widget 生命周期。

**补充说明：**

- 初始化会把 `pWidget->procEvent` 设置为 `xgeXuiButtonEventProc`。
- 初始化会把 `pWidget->procPaint` 设置为 `xgeXuiButtonPaintProc`。
- 默认字体和颜色来自 context theme。

**范例代码：**

```c
xge_xui_widget buttonWidget = xgeXuiWidgetCreate(&ui);
xge_xui_button_t button;
xgeXuiButtonInit(&button, &ui, buttonWidget);
```

**相关 API：**

- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`

---

### xgeXuiButtonUnit

释放按钮控件绑定。

**功能：**

解除按钮对 widget 的事件和绘制回调绑定，并清空按钮对象。

**函数原型：**

```c
XGE_API void xgeXuiButtonUnit(xge_xui_button pButton);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放按钮结构本身，也不释放绑定的 widget、字体或文本字符串。

**补充说明：**

- 只有当 widget 的 `pUser` 仍指向该按钮时，才会清理 widget 回调，避免误清其他控件接管后的回调。

**范例代码：**

```c
xgeXuiButtonUnit(&button);
xgeXuiWidgetFree(buttonWidget);
```

**相关 API：**

- `xgeXuiButtonInit`

---

### xgeXuiButtonSetClick

设置按钮点击回调。

**功能：**

注册按钮点击处理函数和用户数据。按钮在按下后于控件内部释放并仍位于按钮范围内时触发点击。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `procClick`：点击回调，可为 `NULL`。
- `pUser`：传给点击回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存指针并在回调时原样传回。

**补充说明：**

- 清空回调可传入 `procClick = NULL`。
- 点击计数会记录到按钮内部 `iClickCount`。

**范例代码：**

```c
static void on_ok(xge_xui_widget widget, void* user) {
	(void)widget;
	(void)user;
}

xgeXuiButtonSetClick(&button, on_ok, NULL);
```

**相关 API：**

- `xgeXuiButtonEvent`

---

### xgeXuiButtonSetText

设置按钮文字。

**功能：**

设置按钮使用的字体和显示文本，并标记承载 widget 需要重新布局和重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `pFont`：用于绘制按钮文字的字体，可以为 `NULL`。
- `sText`：按钮文字；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

按钮保存 `sText` 的借用指针，不复制字符串。字体资源由调用者或资源系统管理。

**补充说明：**

- 文本必须在按钮绘制期间保持有效。
- 默认文本对齐为水平居中、垂直居中并裁剪。

**范例代码：**

```c
xgeXuiButtonSetText(&button, font, "Start");
```

**相关 API：**

- `xgeXuiButtonSetTextColor`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiButtonSetTextColor

设置按钮文字颜色。

**功能：**

修改按钮文本颜色，并标记承载 widget 需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 颜色使用 XGE 当前统一颜色格式。

**范例代码：**

```c
xgeXuiButtonSetTextColor(&button, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiButtonSetText`
- `xgeXuiButtonSetColors`

---

### xgeXuiButtonSetCheckable

设置按钮可选中状态。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetCheckable(xge_xui_button pButton, int bCheckable);
XGE_API void xgeXuiButtonSetChecked(xge_xui_button pButton, int bChecked);
XGE_API int xgeXuiButtonGetChecked(xge_xui_button pButton);
```

**补充说明：**

- checkable 按钮在成功点击或键盘激活时自动切换 checked。
- `xgeXuiButtonSetChecked` 会隐式开启 checkable。

---

### xgeXuiButtonSetLoading

设置按钮 loading 状态。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetLoading(xge_xui_button pButton, int bLoading);
XGE_API int xgeXuiButtonGetLoading(xge_xui_button pButton);
```

**补充说明：**

- loading 状态下按钮仍会消费鼠标、触控和键盘激活事件，但不会触发 click 回调。
- 默认绘制会在按钮右侧显示轻量三点 loading 标记。

---

### xgeXuiButtonSetSemantic

设置按钮语义样式。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetSemantic(xge_xui_button pButton, int iSemantic);
XGE_API int xgeXuiButtonGetSemantic(xge_xui_button pButton);
```

**补充说明：**

- `iSemantic` 支持 `XGE_XUI_BUTTON_SEMANTIC_DEFAULT`、`XGE_XUI_BUTTON_SEMANTIC_PRIMARY`、`XGE_XUI_BUTTON_SEMANTIC_DANGER`。
- 语义样式会设置按钮默认背景、hover、active、checked 和文本颜色。
- 后续调用 `xgeXuiButtonSetColors` 会回到 default semantic，并使用调用者传入的颜色。

---

### xgeXuiButtonSetIcon

设置 Button 的图标和 icon + text 布局。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetIcon(xge_xui_button pButton, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiButtonSetIconColor(xge_xui_button pButton, uint32_t iColor);
XGE_API void xgeXuiButtonSetIconLayout(xge_xui_button pButton, int iPlacement, float fIconSize, float fGap);
```

**补充说明：**

- `iPlacement` 支持 `XGE_XUI_BUTTON_ICON_LEFT` 和 `XGE_XUI_BUTTON_ICON_RIGHT`。
- `pTexture` 仍由调用者持有，Button 不接管贴图生命周期。
- 默认 icon size 为 16px，默认 gap 为 6px；绘制时会把 icon 和 text 作为一组在 content rect 内居中。
- `xge_xui_button_t::tIconRect` 和 `tTextRect` 保存最近一次 paint 计算出的布局矩形，便于调试和 lab 验证。

---

### xgeXuiButtonSetColors

设置按钮各状态背景颜色。

**功能：**

设置按钮 normal、hover、active、focus、disabled 状态对应的背景颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `iNormal`：普通状态颜色。
- `iHover`：悬停状态颜色。
- `iActive`：按下状态颜色。
- `iFocus`：焦点状态颜色。
- `iDisabled`：禁用状态颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 透明度为 0 的颜色不会绘制背景。
- 实际绘制颜色由当前状态决定。

**范例代码：**

```c
xgeXuiButtonSetColors(&button,
	XGE_COLOR_RGBA(40, 40, 44, 255),
	XGE_COLOR_RGBA(56, 56, 62, 255),
	XGE_COLOR_RGBA(28, 28, 32, 255),
	XGE_COLOR_RGBA(64, 96, 160, 255),
	XGE_COLOR_RGBA(24, 24, 26, 180));
```

**相关 API：**

- `xgeXuiButtonGetState`

---

### xgeXuiButtonGetState

获取按钮当前状态。

**功能：**

返回按钮的当前交互状态，并根据 widget 可见和启用状态刷新 disabled 标记。

**函数原型：**

```c
XGE_API int xgeXuiButtonGetState(xge_xui_button pButton);
```

**参数：**

- `pButton`：按钮对象。

**返回值：**

- 返回 `XGE_XUI_STATE_*` 状态位组合。
- `pButton` 为 `NULL` 时返回 `XGE_XUI_STATE_DISABLED`。

**资源归属：**

不接管外部资源。

**补充说明：**

- 状态可能包含 normal、hover、active、focus、disabled 等位。

**范例代码：**

```c
int state = xgeXuiButtonGetState(&button);
```

**相关 API：**

- `xgeXuiButtonEvent`

---

### xgeXuiButtonEvent

处理按钮事件。

**功能：**

处理鼠标和触控输入，维护按钮 hover/active 状态、焦点、捕获和点击回调。

**函数原型：**

```c
XGE_API int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent);
```

**参数：**

- `pButton`：按钮对象。
- `pEvent`：输入事件。

**返回值：**

- 事件被按钮处理时返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件或不在按钮范围内时返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 按下时会请求 focus 和 pointer capture。
- 释放时如果仍在按钮范围内，会触发点击回调。
- 控件不可见或禁用时不会处理点击。

**范例代码：**

```c
return xgeXuiButtonEvent(&button, event);
```

**相关 API：**

- `xgeXuiButtonSetClick`
- `xgeXuiSetCapture`

---

### xgeXuiButtonEventProc

按钮 widget 事件适配器。

**功能：**

把 widget 事件回调参数转发给 `xgeXuiButtonEvent`，用于挂接到 `xge_xui_widget_t::procEvent`。

**函数原型：**

```c
XGE_API int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：按钮对象指针。

**返回值：**

返回 `xgeXuiButtonEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`，按钮对象会通过自身保存的 `pWidget` 工作。
- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiButtonEventProc;
widget->pUser = &button;
```

**相关 API：**

- `xgeXuiButtonInit`
- `xgeXuiWidgetSetCaptureEvent`

---

### xgeXuiButtonPaintProc

绘制按钮控件。

**功能：**

根据按钮当前状态绘制背景，并在内容区域中绘制按钮文本。

**函数原型：**

```c
XGE_API void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载按钮的 widget。
- `pUser`：按钮对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、按钮、字体或文本资源。

**补充说明：**

- 背景颜色 alpha 为 0 时不会绘制背景。
- 如果字体为空、文本为空或文本首字符为 0，不绘制文字。
- 绘制通过当前 XUI host 完成。

**范例代码：**

```c
xgeXuiButtonPaintProc(button.pWidget, &button);
```

**相关 API：**

- `xgeXuiButtonSetColors`
- `xgeXuiButtonSetText`

---

## API 参考：Label / Image / Input

### xgeXuiLabelInit

初始化文本标签控件。

**功能：**

把 `xge_xui_label_t` 绑定到 widget，设置字体、文本、默认颜色、单缓存和文本测量/绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText);
```

**参数：**

- `pLabel`：标签对象。
- `pWidget`：承载标签的 widget。
- `pFont`：用于测量和绘制的字体，可以为 `NULL`。
- `sText`：标签文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

标签对象、widget 和字体由调用者管理。标签会复制 `sText` 内容，并在 `xgeXuiLabelUnit` 中释放内部文本缓冲。

**补充说明：**

- 初始化会安装 `xgeXuiLabelMeasureProc` 和 `xgeXuiLabelPaintProc`。
- 默认透明、无边框、不可聚焦、IME disabled；背景和边框使用 Widget API 设置。
- 默认颜色为白色，默认禁用色为灰色，默认文本标志为左上对齐并裁剪。
- 默认缓存模式为 `XGE_XUI_CACHE_AUTO`，无 render target 环境自动回退直接绘制。

**范例代码：**

```c
xge_xui_label_t label;
xgeXuiLabelInit(&label, widget, font, "Score");
```

**相关 API：**

- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`

---

### xgeXuiLabelUnit

释放标签控件绑定。

**功能：**

解除标签对 widget 测量和绘制回调的占用，释放标签渲染缓存，并清空标签对象。

**函数原型：**

```c
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或文本字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该标签时才会清理回调。

**范例代码：**

```c
xgeXuiLabelUnit(&label);
```

**相关 API：**

- `xgeXuiLabelInit`

---

### xgeXuiLabelSetText

设置标签文本。

**功能：**

替换标签显示文本，重新测量文本尺寸，标记布局、绘制和标签缓存 dirty。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `sText`：新文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

标签会复制文本内容，不要求传入字符串在绘制期持续有效。

**补充说明：**

- 修改文本会让内容尺寸和单缓存同时失效。

**范例代码：**

```c
xgeXuiLabelSetText(&label, "Ready");
```

**相关 API：**

- `xgeXuiLabelMeasure`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiLabelSetFont

设置标签字体。

**功能：**

替换标签字体，重新测量文本尺寸，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `pFont`：新字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

字体资源由调用者或资源系统管理，标签只保存指针。

**补充说明：**

- 字体为空时测量结果为 0，绘制阶段不会输出文本。

**范例代码：**

```c
xgeXuiLabelSetFont(&label, font);
```

**相关 API：**

- `xgeXuiLabelSetText`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetColor

设置标签文本颜色。

**功能：**

修改标签文本颜色，并标记 widget 和标签缓存需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 颜色 alpha 为 0 时文本由后端按透明色处理。

**范例代码：**

```c
xgeXuiLabelSetColor(&label, XGE_COLOR_RGBA(220, 220, 220, 255));
```

**相关 API：**

- `xgeXuiLabelSetAlign`

---

### xgeXuiLabelSetDisabledColor

设置标签禁用状态文本颜色。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetDisabledColor(xge_xui_label pLabel, uint32_t iColor);
```

禁用色变化会标记标签缓存 dirty 并请求重绘。Label 不处理输入事件，enabled 状态由 Widget 基础设施管理。

---

### xgeXuiLabelSetAlign

设置标签文本对齐方式。

**功能：**

设置标签绘制时传给文本绘制接口的对齐 flags，并强制附加 `XGE_TEXT_CLIP`。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `iTextFlags`：文本对齐和布局标志。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当前实现会始终裁剪到 widget content rect。
- 对齐变化只影响绘制和缓存，不改变文本测量尺寸。

**范例代码：**

```c
xgeXuiLabelSetAlign(&label, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
```

**相关 API：**

- `xgeXuiLabelPaintProc`

---

### xgeXuiLabelSetUnderline

设置标签下划线。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetUnderline(xge_xui_label pLabel, int bUnderline);
```

启用后绘制阶段会附加 `XGE_TEXT_UNDERLINE`。下划线由文本绘制层按每行实际位置绘制，支持对齐和裁剪。

---

### xgeXuiLabelSetCacheMode

设置标签渲染缓存模式。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetCacheMode(xge_xui_label pLabel, int iMode);
```

`iMode` 可取 `XGE_XUI_CACHE_AUTO`、`XGE_XUI_CACHE_OFF`、`XGE_XUI_CACHE_FORCE`。Label 使用单缓存，不为 disabled/hover 等状态预建多份纹理；影响视觉的状态变化会让这一个缓存失效并在下次绘制时惰性重建。

---

### xgeXuiLabelMeasure

测量标签文本尺寸。

**功能：**

通过 XUI host 测量标签当前字体和文本的尺寸。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
```

**参数：**

- `pLabel`：标签对象。

**返回值：**

- 返回文本尺寸。
- 标签、字体或文本为空时返回 `{0, 0}`。

**资源归属：**

不接管外部资源。

**补充说明：**

- 测量能力依赖当前 XUI host 的 `measure_text` 实现。

**范例代码：**

```c
xge_vec2_t size = xgeXuiLabelMeasure(&label);
```

**相关 API：**

- `xgeXuiLabelMeasureProc`

---

### xgeXuiLabelMeasureProc

标签 widget 测量适配器。

**功能：**

把 widget 测量回调转发到 `xgeXuiLabelMeasure`。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：触发测量的 widget。
- `pUser`：标签对象指针。

**返回值：**

返回标签测量尺寸。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。
- 普通用户通常不需要手动调用。

**范例代码：**

```c
widget->procMeasure = xgeXuiLabelMeasureProc;
```

**相关 API：**

- `xgeXuiLabelInit`

---

### xgeXuiLabelPaintProc

绘制标签控件。

**功能：**

把标签文本绘制到 widget content rect 中。缓存可用时绘制缓存纹理；缓存不可用时回退到直接文本绘制。

**函数原型：**

```c
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载标签的 widget。
- `pUser`：标签对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或文本资源。

**补充说明：**

- widget、标签、字体或文本为空时直接返回。
- 直接绘制通过当前 XUI host 完成；缓存重建使用 XGE render target，最终仍通过 XUI host 绘制 image。
- Widget `CONTENT` OwnerDraw 会替换 Label 内容绘制，背景、clip、border、disabled overlay 和 tooltip 仍由 Widget 基础设施处理。

**范例代码：**

```c
xgeXuiLabelPaintProc(widget, &label);
```

**相关 API：**

- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelSetCacheMode`

---

### xgeXuiImageInit

初始化图片控件。

**功能：**

把 `xge_xui_image_t` 绑定到 widget，设置纹理、默认白色 tint 和默认原尺寸居中显示模式。

**函数原型：**

```c
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture);
```

**参数：**

- `pImage`：图片控件对象。
- `pWidget`：承载图片的 widget。
- `pTexture`：要显示的纹理，可以为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

图片控件不接管纹理生命周期，只保存纹理指针。

**补充说明：**

- 初始化会安装图片测量和绘制回调。
- 默认模式为 `XGE_XUI_IMAGE_NATURAL`。

**范例代码：**

```c
xge_xui_image_t image;
xgeXuiImageInit(&image, widget, texture);
```

**相关 API：**

- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`

---

### xgeXuiImageUnit

释放图片控件绑定。

**功能：**

解除图片控件对 widget 测量和绘制回调的占用，并清空图片对象。

**函数原型：**

```c
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget 或纹理资源。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该图片对象时才会清理回调。

**范例代码：**

```c
xgeXuiImageUnit(&image);
```

**相关 API：**

- `xgeXuiImageInit`

---

### xgeXuiImageSetTexture

设置图片纹理。

**功能：**

替换图片控件显示的纹理，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `pTexture`：新纹理，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

图片控件不持有纹理所有权。

**补充说明：**

- 没有 source rect 时，测量尺寸来自纹理宽高。

**范例代码：**

```c
xgeXuiImageSetTexture(&image, texture);
```

**相关 API：**

- `xgeXuiImageSetSource`

---

### xgeXuiImageSetSource

设置图片源区域。

**功能：**

设置纹理源矩形，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `tSrc`：纹理源区域。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当 `tSrc.fW` 和 `tSrc.fH` 大于 0 时，测量尺寸优先使用源区域尺寸。

**范例代码：**

```c
xge_rect_t src = {0.0f, 0.0f, 64.0f, 64.0f};
xgeXuiImageSetSource(&image, src);
```

**相关 API：**

- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetColor

设置图片 tint 颜色。

**功能：**

设置图片绘制颜色，并标记 widget 重绘。

**函数原型：**

```c
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `iColor`：绘制颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 时图片不会绘制。

**范例代码：**

```c
xgeXuiImageSetColor(&image, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiImagePaintProc`

---

### xgeXuiImageSetSourceRect

使用两个顶点设置图片源裁剪区域，语义为 `x1, y1 -> x2, y2`。

```c
XGE_API void xgeXuiImageSetSourceRect(xge_xui_image pImage, float fX1, float fY1, float fX2, float fY2);
```

### xgeXuiImageClearSource

清除图片源裁剪区域，恢复显示整张纹理。

```c
XGE_API void xgeXuiImageClearSource(xge_xui_image pImage);
```

### xgeXuiImageSetTint

设置图片 tint 颜色。新代码优先使用这个语义更明确的名称；`xgeXuiImageSetColor` 保持为同等能力。

```c
XGE_API void xgeXuiImageSetTint(xge_xui_image pImage, uint32_t iColor);
```

---

### xgeXuiImageSetMode

设置图片布局模式。

**功能：**

设置图片在 widget content rect 中的显示方式，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `iMode`：图片模式，例如 natural、stretch、contain、cover、scaleDown、custom。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- `XGE_XUI_IMAGE_NATURAL` 使用源尺寸，按九宫格对齐显示。
- `XGE_XUI_IMAGE_STRETCH` 拉伸填满 content rect。
- `XGE_XUI_IMAGE_CONTAIN` 保持比例完整显示图片。
- `XGE_XUI_IMAGE_COVER` 保持比例填满 content rect，允许超出后被 clip。
- `XGE_XUI_IMAGE_SCALE_DOWN` 原尺寸能放下就原尺寸显示，放不下就 contain。
- `XGE_XUI_IMAGE_CUSTOM` 使用 custom rect 作为目标矩形。
- `XGE_XUI_IMAGE_FIT` 是 `XGE_XUI_IMAGE_CONTAIN` 的别名。

**范例代码：**

```c
xgeXuiImageSetMode(&image, XGE_XUI_IMAGE_FIT);
```

**相关 API：**

- `xgeXuiImagePaintProc`

---

### xgeXuiImageSetAlign

设置 natural、contain、cover、scaleDown 模式下图片在 content rect 中的九宫格对齐位置。

```c
XGE_API void xgeXuiImageSetAlign(xge_xui_image pImage, int iAlignX, int iAlignY);
```

`iAlignX` 使用 `XGE_XUI_ALIGN_START/CENTER/END` 表示 left/center/right，`iAlignY` 使用同一组值表示 top/middle/bottom。

### xgeXuiImageSetCustomRect

使用两个顶点设置 custom 模式下的目标矩形，坐标相对 widget content rect。

```c
XGE_API void xgeXuiImageSetCustomRect(xge_xui_image pImage, float fX1, float fY1, float fX2, float fY2);
```

---

### xgeXuiImageMeasureProc

图片 widget 测量适配器。

**功能：**

返回图片控件的期望尺寸，优先使用 source rect，其次使用纹理尺寸。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：触发测量的 widget。
- `pUser`：图片控件对象指针。

**返回值：**

- 返回图片期望尺寸。
- 图片对象或纹理为空且没有 source rect 时返回 `{0, 0}`。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。

**范例代码：**

```c
xge_vec2_t size = xgeXuiImageMeasureProc(widget, &image);
```

**相关 API：**

- `xgeXuiImageSetSource`
- `xgeXuiImageSetTexture`

---

### xgeXuiImagePaintProc

绘制图片控件。

**功能：**

根据图片模式计算目标矩形，并通过 XUI host 绘制纹理。

**函数原型：**

```c
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载图片的 widget。
- `pUser`：图片控件对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget 或纹理资源。

**补充说明：**

- widget、图片、纹理为空或 tint alpha 为 0 时直接返回。
- 绘制命令使用 `XGE_DRAW_SCREEN_SPACE`。

**范例代码：**

```c
xgeXuiImagePaintProc(widget, &image);
```

**相关 API：**

- `xgeXuiImageSetMode`
- `xgeXuiImageSetColor`

---

### xgeXuiInputInit

初始化单行输入框控件。

**功能：**

初始化输入框内部文本缓冲，绑定 widget，安装事件/绘制回调，并从主题中读取默认字体和颜色。

**函数原型：**

```c
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
```

**参数：**

- `pInput`：输入框对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载输入框的 widget。
- `pFont`：输入框字体；传入 `NULL` 时使用主题字体。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 文本缓冲初始化失败返回对应错误码。

**资源归属：**

输入框内部持有 `xge_xui_text_t` 的缓冲资源，必须用 `xgeXuiInputUnit` 释放。

**补充说明：**

- 初始化会让 widget 可聚焦并启用 clip。
- 输入框依赖系统 IME 事件，不内建完整 IME。

**范例代码：**

```c
xge_xui_input_t input;
xgeXuiInputInit(&input, &ui, widget, font);
```

**相关 API：**

- `xgeXuiInputUnit`
- `xgeXuiTextInit`

---

### xgeXuiInputUnit

释放输入框控件。

**功能：**

解除输入框对 widget 事件和绘制回调的占用，释放内部文本缓冲，并清空输入框对象。

**函数原型：**

```c
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放内部文本缓冲；不释放 widget、字体或输入框结构本身。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该输入框时才会清理回调。

**范例代码：**

```c
xgeXuiInputUnit(&input);
```

**相关 API：**

- `xgeXuiInputInit`

---

### xgeXuiInputSetText

设置输入框文本。

**功能：**

用新文本替换输入框内容，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `sText`：新文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

输入框会复制文本到内部缓冲，不持有 `sText` 指针。

**补充说明：**

- 未初始化输入框会直接返回。
- 如果设置了 `MaxLength`，文本会按 UTF-8 字节边界截断。
- 文本实际变化时会触发 `Change` 回调。

**范例代码：**

```c
xgeXuiInputSetText(&input, "guest");
```

**相关 API：**

- `xgeXuiInputGetText`
- `xgeXuiInputSetChange`
- `xgeXuiInputSetMaxLength`
- `xgeXuiTextSet`

---

### xgeXuiInputGetText

获取输入框文本。

**功能：**

返回输入框当前 UTF-8 文本内容。

**函数原型：**

```c
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象。

**返回值：**

- 返回输入框内部文本指针。
- 输入框为空或文本未初始化时返回空字符串。

**资源归属：**

返回值是借用指针，调用者不能释放，也不应长期保存。

**补充说明：**

- 后续修改输入框文本可能使返回指针失效。

**范例代码：**

```c
const char* value = xgeXuiInputGetText(&input);
```

**相关 API：**

- `xgeXuiInputSetText`

---

### xgeXuiInputSetChange

设置输入框文本变化回调。

**函数原型：**

```c
XGE_API void xgeXuiInputSetChange(xge_xui_input pInput, xge_xui_text_submit_proc procChange, void* pUser);
```

**补充说明：**

- 文本实际变化时触发，包括 `SetText`、键盘输入、剪切、粘贴、删除和 MaxLength 截断。
- 只改变 IME composition 不触发 Change。
- readonly 或 disabled 阻止修改，因此不会触发 Change。

---

### xgeXuiInputSetSubmit

设置输入框提交回调。

**函数原型：**

```c
XGE_API void xgeXuiInputSetSubmit(xge_xui_input pInput, xge_xui_text_submit_proc procSubmit, void* pUser);
```

**补充说明：**

- 输入框获得焦点时，`Enter` 触发 Submit 并消费事件。
- 回调参数 `sText` 为当前输入框文本。

---

### xgeXuiInputSetFilter

设置输入框候选文本过滤回调。

**函数原型：**

```c
typedef int (*xge_xui_input_filter_proc)(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser);
XGE_API void xgeXuiInputSetFilter(xge_xui_input pInput, xge_xui_input_filter_proc procFilter, void* pUser);
```

**补充说明：**

- filter 在文本实际变化后、`Change` 触发前执行；返回非 0 表示接受，返回 0 表示拒绝。
- `sOldText` 是修改前文本，`sNewText` 是经过 `MaxLength` 截断后的候选文本。
- 拒绝时 Input 会回滚到 `sOldText`，递增 `iFilterRejectCount`，不触发 `Change`。
- filter 作用于 `SetText`、普通输入、粘贴、剪切、删除和 clear button 等统一文本变更路径；readonly/disabled 阻止修改时不会调用 filter。

---

### xgeXuiInputSetMaxLength

设置输入框最大文本长度。

**函数原型：**

```c
XGE_API void xgeXuiInputSetMaxLength(xge_xui_input pInput, int iMaxLength);
XGE_API int xgeXuiInputGetMaxLength(xge_xui_input pInput);
```

**补充说明：**

- `iMaxLength <= 0` 表示不限制长度。
- 长度单位沿用 `xge_xui_text_t::iSize` 的 UTF-8 字节数，并保证截断不会落在 UTF-8 continuation byte 中间。
- 设置 MaxLength 时如果当前文本超长，会立即截断并触发 Change。

---

### xgeXuiInputSetError

设置输入框错误状态。

**函数原型：**

```c
XGE_API void xgeXuiInputSetErrorChange(xge_xui_input pInput, xge_xui_input_error_proc procError, void* pUser);
XGE_API void xgeXuiInputSetError(xge_xui_input pInput, int bError);
XGE_API int xgeXuiInputGetError(xge_xui_input pInput);
XGE_API void xgeXuiInputSetErrorColors(xge_xui_input pInput, uint32_t iBackground, uint32_t iBorder);
```

**补充说明：**

- Error 是 Input 私有状态，不修改公共 `XGE_XUI_STATE_*` 位。
- Error 状态优先绘制错误背景和错误边框，但不改变输入、选择、复制、提交等行为。
- Input 不绘制盒子外错误提示；设置错误态或清除错误态时通过 `procError` 通知，提示文案、显示位置和交互方式由业务层决定。

---

### xgeXuiInputSetTextAlign

设置 Input 文本水平对齐方式。

**函数原型：**

```c
XGE_API void xgeXuiInputSetTextAlign(xge_xui_input pInput, int iAlign);
XGE_API int xgeXuiInputGetTextAlign(xge_xui_input pInput);
```

**补充说明：**

- `iAlign` 支持 `XGE_XUI_INPUT_TEXT_ALIGN_LEFT`、`XGE_XUI_INPUT_TEXT_ALIGN_CENTER`、`XGE_XUI_INPUT_TEXT_ALIGN_RIGHT`。
- 默认左对齐。
- 文本未超出内容区时，文本、placeholder、selection、cursor、IME candidate 和鼠标命中会一起按对齐方式偏移。
- 文本超出内容区后仍使用水平滚动逻辑，优先保证光标可见。
- 右对齐适合配合 trailing text decoration，例如 `px`、`%`、`ms`。

---

### xgeXuiInputDecorationAdd

添加、更新或删除 Input 左右装饰区项。

**函数原型：**

```c
XGE_API xge_xui_input_decoration xgeXuiInputDecorationAdd(xge_xui_input pInput, int iSide, const xge_xui_input_decoration_desc_t* pDesc);
XGE_API void xgeXuiInputDecorationSet(xge_xui_input pInput, xge_xui_input_decoration pDecoration, const xge_xui_input_decoration_desc_t* pDesc);
XGE_API void xgeXuiInputDecorationRemove(xge_xui_input pInput, xge_xui_input_decoration pDecoration);
XGE_API void xgeXuiInputDecorationClear(xge_xui_input pInput, int iSide);
XGE_API xge_rect_t xgeXuiInputDecorationGetRect(xge_xui_input pInput, xge_xui_input_decoration pDecoration);
```

**补充说明：**

- 装饰区分为 `XGE_XUI_INPUT_DECORATION_SIDE_LEADING` 和 `XGE_XUI_INPUT_DECORATION_SIDE_TRAILING`。
- 装饰类型支持 `ICON`、`TEXT`、`TEXTURE`、`CLEAR`、`CUSTOM_PAINT`。
- 可见策略支持 always、when not empty、when focused、when focused and not empty。
- 装饰项会自动参与文本 padding 计算；不可见项不占宽、不绘制、不命中。
- `CLEAR` 默认可点击；其他装饰项设置 `procClick` 后可点击。
- `xgeXuiInputDecorationGetRect` 返回最近一次布局矩形，会在必要时触发装饰区布局刷新。

---

### xgeXuiInput Decoration Convenience Wrappers

旧 clear/icon API 不再作为 Input 装饰区的设计入口。新代码和新范例应直接使用 `xgeXuiInputDecorationAdd` 等 API。

**函数原型：**

```c
XGE_API void xgeXuiInputSetClearButton(xge_xui_input pInput, int bEnabled);
XGE_API int xgeXuiInputGetClearButton(xge_xui_input pInput);
XGE_API xge_rect_t xgeXuiInputGetClearRect(xge_xui_input pInput);
XGE_API void xgeXuiInputSetClearColors(xge_xui_input pInput, uint32_t iColor, uint32_t iHoverColor);
XGE_API void xgeXuiInputSetIcons(xge_xui_input pInput, int iPrefixIcon, int iSuffixIcon);
XGE_API void xgeXuiInputSetIconColor(xge_xui_input pInput, uint32_t iColor);
XGE_API xge_rect_t xgeXuiInputGetPrefixIconRect(xge_xui_input pInput);
XGE_API xge_rect_t xgeXuiInputGetSuffixIconRect(xge_xui_input pInput);
```

**补充说明：**

- 这些 API 内部不再维护第二套 clear/icon 状态。
- `xgeXuiInputSetClearButton` 会增删 trailing `CLEAR` decoration。
- `xgeXuiInputSetIcons` 会增删 leading/trailing `ICON` decoration。

---

### xgeXuiInputSetPassword

设置输入框密码模式。

**函数原型：**

```c
XGE_API void xgeXuiInputSetPassword(xge_xui_input pInput, int bPassword);
```

**补充说明：**

- 密码模式绘制为掩码文本，内部仍保存原始文本。
- 密码模式下 `Ctrl+C` / `Ctrl+X` 会被消费但不会写入剪贴板，也不会修改文本。
- 密码模式下非 ASCII 文本输入会被消费且不修改文本。
- 密码模式获得焦点或收到 IME 事件时会禁用 IME 并清空 composition，失焦时恢复进入密码模式前的 IME 状态。

---

### xgeXuiInputSetReadonly

设置输入框只读模式。

**函数原型：**

```c
XGE_API void xgeXuiInputSetReadonly(xge_xui_input pInput, int bReadonly);
```

**补充说明：**

- readonly 下仍允许焦点、光标移动、鼠标选择、`Ctrl+A` 全选和复制。
- readonly 下文本输入、IME 提交、粘贴、剪切、Backspace、Delete、clear button 和默认菜单删除都不会修改文本，也不会触发 `Change`。
- readonly 不等同于 disabled；disabled 会影响绘制和交互可用性。

---

### xgeXuiInputSetFont

设置输入框字体。

**功能：**

替换输入框字体，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `pFont`：新字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

字体由调用者或资源系统管理，输入框只保存指针。

**补充说明：**

- 候选框位置和游标位置依赖字体测量结果。

**范例代码：**

```c
xgeXuiInputSetFont(&input, font);
```

**相关 API：**

- `xgeXuiInputGetCandidateRect`

---

### xgeXuiInputSetColors

设置输入框颜色。

**功能：**

设置输入框文本、背景、焦点背景和游标颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `iText`：文本颜色。
- `iBackground`：普通背景颜色。
- `iFocus`：获得焦点时的背景颜色。
- `iCursor`：游标颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- selection 颜色来自主题初始化值，当前函数不修改 selection 颜色。

**范例代码：**

```c
xgeXuiInputSetColors(&input, textColor, bgColor, focusColor, cursorColor);
```

**相关 API：**

- `xgeXuiInputPaintProc`

---

### xgeXuiInputSetSelection

设置输入框选择区。

**功能：**

设置输入框内部文本选择区，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `iStart`：选择起点 UTF-8 字节偏移。
- `iEnd`：选择终点 UTF-8 字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 未初始化输入框会直接返回。
- 偏移会被底层文本对象夹到合法 UTF-8 边界。

**范例代码：**

```c
xgeXuiInputSetSelection(&input, 0, 4);
```

**相关 API：**

- `xgeXuiInputGetSelection`
- `xgeXuiTextSetSelection`

---

### xgeXuiInputGetSelection

获取输入框选择区。

**功能：**

读取输入框当前选择区，输出 UTF-8 字节偏移范围。

**函数原型：**

```c
XGE_API void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `pStart`：接收选择起点，可为 `NULL`。
- `pEnd`：接收选择终点，可为 `NULL`。

**返回值：**

无。

**资源归属：**

输出参数由调用者持有。

**补充说明：**

- 输入框为空时输出 0 到 0。

**范例代码：**

```c
int start;
int end;
xgeXuiInputGetSelection(&input, &start, &end);
```

**相关 API：**

- `xgeXuiInputSetSelection`

---

### xgeXuiInputGetCandidateRect

获取 IME 候选框参考区域。

**功能：**

根据输入框内容、字体测量和当前游标位置，计算系统 IME 候选框应锚定的矩形。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象。

**返回值：**

- 返回候选框参考矩形。
- 输入框或 widget 为空时返回全 0 矩形。

**资源归属：**

不接管外部资源。

**补充说明：**

- candidate rect 统一以当前 content rect 为基准：`x/y` 对齐光标位置和内容区顶部，`w` 固定为 1px，`h` 等于内容区高度。
- rect 会夹在输入框内容区范围内，并扣除横向滚动偏移。
- 平台后端优先读取当前 context 的 `xgeXuiGetImeCandidateRect`；本函数仍用于 Input 自身计算或自定义宿主直接查询。
- password 模式会禁用 IME，不应显示候选框。

**范例代码：**

```c
xge_rect_t imeRect = xgeXuiInputGetCandidateRect(&input);
```

**相关 API：**

- `xgeXuiTextGetCursor`
- `xgeXuiInputEvent`

---

### xgeXuiInputEvent

处理输入框事件。

**功能：**

处理鼠标、触控、文本、IME 和键盘事件，维护焦点、选择区、游标和文本内容。

**函数原型：**

```c
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
```

**参数：**

- `pInput`：输入框对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 鼠标/触控按下会设置焦点、计算游标并启动选择。
- 文本和 IME 事件只在输入框获得焦点后处理。
- 支持 Backspace、Delete、Left、Right 的基础编辑行为。

**范例代码：**

```c
int handled = xgeXuiInputEvent(&input, event);
```

**相关 API：**

- `xgeXuiTextInputEvent`
- `xgeXuiInputGetCandidateRect`

---

### xgeXuiInputEventProc

输入框 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiInputEvent`。

**函数原型：**

```c
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：输入框对象指针。

**返回值：**

返回 `xgeXuiInputEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。
- 普通用户通常不需要手动调用。

**范例代码：**

```c
widget->procEvent = xgeXuiInputEventProc;
```

**相关 API：**

- `xgeXuiInputInit`

---

### xgeXuiInputPaintProc

绘制输入框控件。

**功能：**

绘制输入框背景、选择区、文本、IME composition 和焦点游标。

**函数原型：**

```c
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载输入框的 widget。
- `pUser`：输入框对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、输入框或字体资源。

**补充说明：**

- 背景颜色根据输入框是否获得焦点切换。
- composition 会绘制在候选框参考位置之后。
- 游标只在输入框获得焦点且游标颜色非透明时绘制。

**范例代码：**

```c
xgeXuiInputPaintProc(widget, &input);
```

**相关 API：**

- `xgeXuiInputSetColors`
- `xgeXuiInputGetCandidateRect`

---

## API 参考：TextEdit 标准策略

TextEdit 是多行文本编辑控件，基础 API 包括：

```c
XGE_API void xgeXuiTextEditSetReadonly(xge_xui_text_edit pEdit, int bReadonly);
XGE_API int xgeXuiTextEditUndo(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditRedo(xge_xui_text_edit pEdit);
XGE_API void xgeXuiTextEditSetFindHighlights(xge_xui_text_edit pEdit, const xge_xui_text_edit_highlight_t* arrHighlights, int iCount);
XGE_API void xgeXuiTextEditSetLineNumbers(xge_xui_text_edit pEdit, int bEnabled, float fWidth);
XGE_API void xgeXuiTextEditSetReserveColors(xge_xui_text_edit pEdit, uint32_t iFindHighlight, uint32_t iLineNumberText, uint32_t iLineNumberBackground);
```

- readonly 下仍允许焦点、光标移动、选择和复制；文本输入、IME、粘贴、剪切、Backspace、Delete 和 Enter 不会修改文本。
- 当前 undo/redo policy 为“单次编辑操作一条快照”：普通文本输入、Enter、Tab、粘贴、剪切、删除各记录一次 undo；新编辑会清空 redo 栈。
- `Tab` 默认插入字面量 `\t`，并进入 undo 栈；readonly 下 Tab 不修改文本。
- find highlight 预留接受外部 range 数组，TextEdit 不接管数组生命周期，也不负责执行查找算法。
- line number 预留绘制左侧 gutter，并在 paint 后恢复 widget content rect；`fWidth <= 0` 时使用默认宽度。
- `examples/xui_text_edit_standard_lab` 覆盖 readonly selection、undo/redo 单步策略、redo clear、Tab 输入策略、find highlight 和 line number 预留。

---

## API 参考：Toggle / Slider / Progress

### xgeXuiSliderInit

初始化滑块控件。

**功能：**

把 `xge_xui_slider_t` 绑定到 widget，设置默认范围、颜色、事件回调和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pSlider`：滑块控件对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载滑块的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

控件对象、context 和 widget 均由调用者持有。

**补充说明：**

- 默认范围为 0 到 1，默认值为 0。
- 初始化会让 widget 可聚焦。

**范例代码：**

```c
xge_xui_slider_t slider;
xgeXuiSliderInit(&slider, &ui, widget);
```

**相关 API：**

- `xgeXuiSliderUnit`
- `xgeXuiSliderSetRange`

---

### xgeXuiSliderUnit

释放滑块控件绑定。

**功能：**

解除滑块对 widget 事件和绘制回调的占用，并清空控件对象。

**函数原型：**

```c
XGE_API void xgeXuiSliderUnit(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget 或外部资源。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该控件时才会清理回调。

**范例代码：**

```c
xgeXuiSliderUnit(&slider);
```

**相关 API：**

- `xgeXuiSliderInit`

---

### xgeXuiSliderSetChange

设置滑块变化回调。

**功能：**

注册滑块值变化回调和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `procChange`：值变化回调，可以为 `NULL`。
- `pUser`：传给回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存并回传指针。

**补充说明：**

- 拖动或点击导致值变化时会触发回调。

**范例代码：**

```c
xgeXuiSliderSetChange(&slider, on_slider, user);
```

**相关 API：**

- `xgeXuiSliderEvent`

---

### xgeXuiSliderSetRange

设置滑块范围。

**功能：**

设置滑块最小值和最大值，并把当前值夹到新范围内。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `fMin`：范围最小值。
- `fMax`：范围最大值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 如果 `fMax < fMin` 会自动交换。
- 如果两者相等，最大值会调整为 `fMin + 1.0f`。

**范例代码：**

```c
xgeXuiSliderSetRange(&slider, 0.0f, 100.0f);
```

**相关 API：**

- `xgeXuiSliderSetValue`

---

### xgeXuiSliderSetValue

设置滑块当前值。

**功能：**

设置滑块值，并自动夹到当前范围内。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `fValue`：目标值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 程序主动设置值不会触发 change 回调。

**范例代码：**

```c
xgeXuiSliderSetValue(&slider, 50.0f);
```

**相关 API：**

- `xgeXuiSliderGetValue`

---

### xgeXuiSliderGetValue

获取滑块当前值。

**功能：**

返回滑块当前数值。

**函数原型：**

```c
XGE_API float xgeXuiSliderGetValue(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象。

**返回值：**

- 返回当前值。
- `pSlider` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值位于当前范围内。

**范例代码：**

```c
float value = xgeXuiSliderGetValue(&slider);
```

**相关 API：**

- `xgeXuiSliderSetValue`

---

### xgeXuiSliderSetColors

设置滑块颜色。

**功能：**

设置轨道、填充、滑块、焦点和禁用颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `iTrack`：轨道颜色。
- `iFill`：已填充轨道颜色。
- `iKnob`：滑块颜色。
- `iFocus`：焦点背景颜色。
- `iDisabled`：禁用轨道颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 禁用状态下只绘制禁用轨道。

**范例代码：**

```c
xgeXuiSliderSetColors(&slider, track, fill, knob, focus, disabled);
```

**相关 API：**

- `xgeXuiSliderPaintProc`

---

### xgeXuiSliderGetState

获取滑块交互状态。

**功能：**

返回滑块当前交互状态，并根据 widget 可见和启用状态刷新 disabled 标记。

**函数原型：**

```c
XGE_API int xgeXuiSliderGetState(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象。

**返回值：**

- 返回 `XGE_XUI_STATE_*` 状态位组合。
- `pSlider` 为 `NULL` 时返回 `XGE_XUI_STATE_DISABLED`。

**资源归属：**

不接管外部资源。

**补充说明：**

- active 状态通常表示正在拖动。

**范例代码：**

```c
int state = xgeXuiSliderGetState(&slider);
```

**相关 API：**

- `xgeXuiSliderEvent`

---

### xgeXuiSliderEvent

处理滑块事件。

**功能：**

处理鼠标和触控事件，维护 hover/active 状态、焦点、捕获，并根据指针位置更新滑块值。

**函数原型：**

```c
XGE_API int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent);
```

**参数：**

- `pSlider`：滑块控件对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 指针按下和拖动时会根据 X 坐标更新值。
- 用户交互导致值变化时会触发 change 回调。

**范例代码：**

```c
return xgeXuiSliderEvent(&slider, event);
```

**相关 API：**

- `xgeXuiSliderSetChange`
- `xgeXuiSetCapture`

---

### xgeXuiSliderEventProc

滑块 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiSliderEvent`。

**函数原型：**

```c
XGE_API int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：滑块控件对象指针。

**返回值：**

返回 `xgeXuiSliderEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiSliderEventProc;
```

**相关 API：**

- `xgeXuiSliderInit`

---

### xgeXuiSliderPaintProc

绘制滑块控件。

**功能：**

绘制滑块轨道、填充段、焦点背景和滑块按钮。

**函数原型：**

```c
XGE_API void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载滑块的 widget。
- `pUser`：滑块控件对象指针。

**返回值：**

无。

**资源归属：**

不接管任何参数。

**补充说明：**

- 轨道高度最多 6 像素。
- 滑块尺寸取 content 高度并限制到 6 到 18 像素。

**范例代码：**

```c
xgeXuiSliderPaintProc(widget, &slider);
```

**相关 API：**

- `xgeXuiSliderSetColors`

---

### xgeXuiProgressInit

初始化进度条控件。

**功能：**

把 `xge_xui_progress_t` 绑定到 widget，设置默认范围、颜色、文本标志和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget);
```

**参数：**

- `pProgress`：进度条控件对象。
- `pWidget`：承载进度条的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

控件对象和 widget 均由调用者持有。

**补充说明：**

- 默认范围为 0 到 1。
- 默认文本为空，设置文本后才绘制文字。

**范例代码：**

```c
xge_xui_progress_t progress;
xgeXuiProgressInit(&progress, widget);
```

**相关 API：**

- `xgeXuiProgressUnit`
- `xgeXuiProgressSetValue`

---

### xgeXuiProgressUnit

释放进度条控件绑定。

**功能：**

解除进度条对 widget 绘制回调的占用，并清空控件对象。

**函数原型：**

```c
XGE_API void xgeXuiProgressUnit(xge_xui_progress pProgress);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或文本字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该控件时才会清理回调。

**范例代码：**

```c
xgeXuiProgressUnit(&progress);
```

**相关 API：**

- `xgeXuiProgressInit`

---

### xgeXuiProgressSetRange

设置进度条范围。

**功能：**

设置进度条最小值和最大值，并把当前值夹到新范围内。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `fMin`：范围最小值。
- `fMax`：范围最大值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 如果 `fMax < fMin` 会自动交换。
- 如果两者相等，最大值会调整为 `fMin + 1.0f`。

**范例代码：**

```c
xgeXuiProgressSetRange(&progress, 0.0f, 100.0f);
```

**相关 API：**

- `xgeXuiProgressSetValue`

---

### xgeXuiProgressSetValue

设置进度条当前值。

**功能：**

设置进度值，并自动夹到当前范围内。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `fValue`：目标值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 值变化后会标记 widget 重绘。

**范例代码：**

```c
xgeXuiProgressSetValue(&progress, 75.0f);
```

**相关 API：**

- `xgeXuiProgressGetValue`

---

### xgeXuiProgressGetValue

获取进度条当前值。

**功能：**

返回进度条当前数值。

**函数原型：**

```c
XGE_API float xgeXuiProgressGetValue(xge_xui_progress pProgress);
```

**参数：**

- `pProgress`：进度条控件对象。

**返回值：**

- 返回当前值。
- `pProgress` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值位于当前范围内。

**范例代码：**

```c
float value = xgeXuiProgressGetValue(&progress);
```

**相关 API：**

- `xgeXuiProgressSetValue`

---

### xgeXuiProgressSetText

设置进度条文本。

**功能：**

设置进度条中央显示的文本和字体，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `pFont`：文本字体，可以为 `NULL`。
- `sText`：显示文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

控件保存 `sText` 的借用指针，不复制字符串。

**补充说明：**

- 文本默认居中绘制并裁剪。

**范例代码：**

```c
xgeXuiProgressSetText(&progress, font, "75%");
```

**相关 API：**

- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetTextColor

设置进度条文本颜色。

**功能：**

设置进度条文本绘制颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 仅影响进度条文本，不影响轨道和填充颜色。

**范例代码：**

```c
xgeXuiProgressSetTextColor(&progress, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiProgressSetText`

---

### xgeXuiProgressSetColors

设置进度条颜色。

**功能：**

设置进度条轨道颜色和填充颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `iTrack`：轨道颜色。
- `iFill`：填充颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 的轨道或填充不会绘制。

**范例代码：**

```c
xgeXuiProgressSetColors(&progress, trackColor, fillColor);
```

**相关 API：**

- `xgeXuiProgressPaintProc`

---

### xgeXuiProgressPaintProc

绘制进度条控件。

**功能：**

绘制进度条轨道、按当前进度裁剪的填充区域，以及可选居中文本。

**函数原型：**

```c
XGE_API void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载进度条的 widget。
- `pUser`：进度条控件对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或文本资源。

**补充说明：**

- 填充宽度按 `(value - min) / (max - min)` 计算。

**范例代码：**

```c
xgeXuiProgressPaintProc(widget, &progress);
```

**相关 API：**

- `xgeXuiProgressSetValue`
- `xgeXuiProgressSetColors`

---

## API 参考：Panel / Window / Viewport Rebuild

### xgeXuiPanelInit

初始化面板控件。

**功能：**

把 `xge_xui_panel_t` 绑定到 widget，通过 widget 基础绘制设置默认背景/边框，并设置标题绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
```

**参数：**

- `pPanel`：面板对象。
- `pWidget`：承载面板的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

面板对象和 widget 均由调用者持有。

**补充说明：**

- 初始化后标题为空；背景和边框由 widget 基础绘制负责。
- 面板不主动管理子控件，只提供容器背景和可选标题。

**范例代码：**

```c
xge_xui_panel_t panel;
xgeXuiPanelInit(&panel, widget);
```

**相关 API：**

- `xgeXuiPanelUnit`
- `xgeXuiPanelSetTitle`

---

### xgeXuiPanelUnit

释放面板控件绑定。

**功能：**

解除面板对 widget 绘制回调的占用，并清空面板对象。

**函数原型：**

```c
XGE_API void xgeXuiPanelUnit(xge_xui_panel pPanel);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或标题字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该面板时才会清理回调。

**范例代码：**

```c
xgeXuiPanelUnit(&panel);
```

**相关 API：**

- `xgeXuiPanelInit`

---

### xgeXuiPanelSetBackground

设置面板背景颜色。

**功能：**

修改面板背景颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iColor`：背景颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 时不会绘制背景。

**范例代码：**

```c
xgeXuiPanelSetBackground(&panel, XGE_COLOR_RGBA(32, 38, 46, 255));
```

**相关 API：**

- `xgeXuiPanelPaintProc`

---

### xgeXuiPanelSetTitle

设置面板标题。

**功能：**

设置面板标题字体和标题文本，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `pFont`：标题字体，可以为 `NULL`。
- `sTitle`：标题文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

面板保存标题字符串借用指针，不复制字符串。

**补充说明：**

- 标题绘制在 widget content rect 中。

**范例代码：**

```c
xgeXuiPanelSetTitle(&panel, font, "Settings");
```

**相关 API：**

- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`

---

### xgeXuiPanelSetTitleColor

设置面板标题颜色。

**功能：**

修改标题文本颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iColor`：标题颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 仅影响标题文本，不影响背景。

**范例代码：**

```c
xgeXuiPanelSetTitleColor(&panel, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiPanelSetTitle`

---

### xgeXuiPanelSetTitleAlign

设置面板标题对齐方式。

**功能：**

设置标题文本绘制 flags，并强制附加 `XGE_TEXT_CLIP`。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iTextFlags`：标题文本 flags。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 标题始终裁剪在 content rect 内。

**范例代码：**

```c
xgeXuiPanelSetTitleAlign(&panel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP);
```

**相关 API：**

- `xgeXuiPanelPaintProc`

---

### xgeXuiPanelSetClip

设置面板裁剪。

**功能：**

设置面板自身的 clip 标记，并同步到承载 widget。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `bClip`：非 0 启用裁剪，0 禁用裁剪。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 裁剪行为最终由 widget 的 `XGE_XUI_WIDGET_CLIP` 标志控制。

**范例代码：**

```c
xgeXuiPanelSetClip(&panel, 1);
```

**相关 API：**

- `xgeXuiWidgetSetClip`

---

### xgeXuiPanelPaintProc

绘制面板控件。

**功能：**

绘制面板背景和可选标题文本。

**函数原型：**

```c
XGE_API void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载面板的 widget。
- `pUser`：面板对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或标题字符串。

**补充说明：**

- widget 或面板为空时直接返回。

**范例代码：**

```c
xgeXuiPanelPaintProc(widget, &panel);
```

**相关 API：**

- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`

---

### Viewport 系列重构状态

ScrollModel、ScrollFrame、ScrollView、Popup、ListView、TreeView、TableView、ColorPicker 已按新的 viewport 架构落地。

VirtualView、PropertyGrid、Menu、ComboBox、TextEdit 正在继续重构。TableGrid、textarea/enum/color/date/time/datetime 标准编辑器、picker/file/image 标准入口和 XSON `tableGrid` 已恢复，true immediate 模式继续按专属 spec 跟踪。旧 `ScrollViewBase` / `VirtualScrollViewBase` API 不再作为新实现口径，未恢复控件源码已从编译入口隔离。

当前权威设计见：

- [Viewport / Scroll](../xui/scrollview.md)
- [Viewport Refactor Spec](../xui/viewport-refactor-spec.md)
- [Popup](../xui/popup.md)
- [ListView](../xui/listview.md)
- [TreeView](../xui/treeview.md)
- [TableView](../xui/tableview.md)
- [TableGrid](../xui/tablegrid.md)

已恢复的 XSON 类型包括 `scroll` / `scrollView` / `popup` / `listView` / `treeView` / `tableView` / `tableGrid` / `dockLayout`。

加载仍处于隔离状态的类型会返回明确不可用错误，不能回落到旧实现。

## API 参考：Tabs 标准能力

Tabs 支持外部数组驱动的 item 文本、enabled 状态、dirty 标记和 icon 槽位。Tabs 不接管这些数组的生命周期，调用方需要保证数组在控件使用期间有效。

```c
XGE_API void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount);
XGE_API void xgeXuiTabsSetEnabledItems(xge_xui_tabs pTabs, const int* arrEnabled, int iCount);
XGE_API void xgeXuiTabsSetDirtyItems(xge_xui_tabs pTabs, const int* arrDirty, int iCount);
XGE_API void xgeXuiTabsSetIcons(xge_xui_tabs pTabs, const xge_texture* arrIcons, const xge_rect_t* arrSrc, int iCount);
```

- disabled tab 不可被鼠标或键盘选中。
- dirty tab 会绘制小型状态标记。
- icon 槽位为文本预留空间；`arrIcons[i]` 为 `NULL` 时只保留布局能力。

Tabs 关闭按钮和可滚动 tab strip：

```c
XGE_API void xgeXuiTabsSetClose(xge_xui_tabs pTabs, xge_xui_select_proc procClose, int bCloseButtons, void* pUser);
XGE_API void xgeXuiTabsSetScrollable(xge_xui_tabs pTabs, int bScrollable);
XGE_API void xgeXuiTabsSetScroll(xge_xui_tabs pTabs, float fScrollX);
XGE_API float xgeXuiTabsGetScroll(xge_xui_tabs pTabs);
```

- close button 点击只触发 `procClose`，不触发 select 回调。
- `SetScroll` 会把 `scrollX` clamp 到合法范围。
- Left/Right 键会跳过 disabled tab，选中不可见 tab 时会自动调整 `scrollX`。

---

### 旧 VirtualView API 已隔离

旧 VirtualScrollViewBase 文档已移除。ListView 已按 [ListView](../xui/listview.md) 恢复；TreeView 已按 [TreeView](../xui/treeview.md) 恢复；TableView 已按 [TableView](../xui/tableview.md) 恢复；TableGrid 已按 [TableGrid](../xui/tablegrid.md) 恢复常用编辑器；VirtualView、PropertyGrid 必须按 [Viewport / Scroll](../xui/scrollview.md) 和 [Viewport Refactor Spec](../xui/viewport-refactor-spec.md) 重新建立 API，不继续沿用旧 base 命名。

### xgeXuiMsgBoxInit

Binds a MsgBox object to a widget and installs its event and paint callbacks.

```c
XGE_API int xgeXuiMsgBoxInit(xge_xui_msg_box pBox, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiMsgBoxUnit(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetText(xge_xui_msg_box pBox, xge_font pFont, const char* sTitle, const char* sMessage);
XGE_API void xgeXuiMsgBoxSetType(xge_xui_msg_box pBox, int iType);
XGE_API void xgeXuiMsgBoxSetIconTexture(xge_xui_msg_box pBox, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeXuiMsgBoxSetButtons(xge_xui_msg_box pBox, int iButtons);
XGE_API void xgeXuiMsgBoxSetCustomButtons(xge_xui_msg_box pBox, xvalue arrButtons);
XGE_API void xgeXuiMsgBoxSetResult(xge_xui_msg_box pBox, xge_xui_select_proc procResult, void* pUser);
XGE_API void xgeXuiMsgBoxSetModal(xge_xui_msg_box pBox, int bModal);
XGE_API void xgeXuiMsgBoxSetOpen(xge_xui_msg_box pBox, int bOpen);
XGE_API int xgeXuiMsgBoxIsOpen(xge_xui_msg_box pBox);
XGE_API int xgeXuiMsgBoxGetResult(xge_xui_msg_box pBox);
XGE_API void xgeXuiMsgBoxSetColors(xge_xui_msg_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iMessage, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiMsgBoxEvent(xge_xui_msg_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiMsgBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiMsgBoxPaintProc(xge_xui_widget pWidget, void* pUser);
```

### xgeXuiInputBoxInit

Binds an InputBox object to a widget and uses the same window-based overlay behavior as MsgBox.

```c
XGE_API int xgeXuiInputBoxInit(xge_xui_input_box pBox, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
XGE_API void xgeXuiInputBoxUnit(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetText(xge_xui_input_box pBox, xge_font pFont, const char* sTitle, const char* sPrompt, const char* sInitial);
XGE_API void xgeXuiInputBoxSetResult(xge_xui_input_box pBox, xge_xui_text_submit_proc procResult, void* pUser);
XGE_API void xgeXuiInputBoxSetModal(xge_xui_input_box pBox, int bModal);
XGE_API void xgeXuiInputBoxSetOpen(xge_xui_input_box pBox, int bOpen);
XGE_API int xgeXuiInputBoxIsOpen(xge_xui_input_box pBox);
XGE_API int xgeXuiInputBoxGetResultCode(xge_xui_input_box pBox);
XGE_API char* xgeXuiInputBoxGetResult(xge_xui_input_box pBox);
XGE_API void xgeXuiInputBoxSetColors(xge_xui_input_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iPrompt, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText);
XGE_API int xgeXuiInputBoxEvent(xge_xui_input_box pBox, const xge_event_t* pEvent);
XGE_API int xgeXuiInputBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputBoxPaintProc(xge_xui_widget pWidget, void* pUser);
```

## 生命周期与所有权

XUI context 由调用者持有，使用 `xgeXuiInit` / `xgeXuiUnit` 管理。Widget 和控件对象也由调用者持有，XUI 只维护树关系和回调。

Host 回调和 `pUser` 由宿主管理，XUI 不接管其生命周期。控件引用的 font、texture 等 XGE 资源必须在控件绘制期间保持有效。

## 线程约束

XUI 是 retained-mode GUI，建议在主 UI 线程进行事件派发、布局更新和 paint。不要多个线程同时修改同一个 widget tree。

异步资源完成后应回到 UI 线程更新控件状态，并请求 refresh。

## 后端差异

XUI 本身不绑定 GL、Sokol 或 XGE 私有状态。当前 XGE 内孵化版本使用 XGE host 绘制；未来剥离到 XUI 仓库后，可以通过其他 host 接入。

IME、触控、鼠标和候选框位置依赖平台后端能力。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 控件修改后不刷新 | 没有标记 dirty 或请求刷新 | 调用 `xgeXuiWidgetMarkPaint` 或 `xgeXuiRefreshRequest`。 |
| 输入框 IME 候选框位置不对 | 宿主没有读取当前焦点 candidate rect | 使用 `xgeXuiGetImeCandidateRect` 给系统 IME；自定义文本控件通过 `xgeXuiWidgetSetImeCandidateRect` 注册解析器。 |
| 布局频繁抖动 | 多次修改 widget 未批处理 | 用 `xgeXuiLayoutBatchBegin` / `xgeXuiLayoutBatchEnd` 包住批量修改。 |

## 相关示例

- `examples/xui_bridge`
- `examples\xui_bridge\build.bat`
