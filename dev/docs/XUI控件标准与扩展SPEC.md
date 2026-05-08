# XUI 控件标准与扩展 SPEC

本文档跟踪 XUI 控件标准化、旧控件维护、新控件扩展、内置风格升级和 XSON 控件声明式能力的开发进度。

关联设计文档：

- `dev/docs/XUI Widget V2基础设计.md`
- `dev/docs/XUI Widget V2基础SPEC.md`
- `dev/docs/XUI控件标准与扩展设计.md`
- `dev/docs/XUI轻量控件与布局SPEC.md`
- `dev/docs/XUI声明式界面与样式SPEC.md`

> 2026-05-07 口径更新：本文历史 `[x]` 只代表旧基础层上的实现/验证；控件成熟度必须在 Widget V2 基础 SPEC 中重新验收。

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

完成口径：

- 设计任务：文档更新完成，并明确边界。
- API 任务：公共声明、实现和最小调用示例齐备。
- 控件任务：至少覆盖创建、属性、事件、绘制、销毁。
- 交互任务：鼠标、触摸、键盘、焦点、捕获丢失路径覆盖。
- Overlay 任务：open/close、outside click、Escape、focus restore、z-order 覆盖。
- XSON 任务：字段解析、错误路径、资源释放和失败回滚覆盖。
- 风格任务：默认 token、控件状态色、示例截图或人工验证记录齐备。
- 性能任务：说明热路径不做重复字符串查找、全树重算或大量 widget 创建。

## 总体里程碑

- [x] 梳理当前 XUI 已有控件、布局、Page Loader 和 XSON 支持范围。
- [x] 生成控件标准与扩展设计文档。
- [x] 生成可跟踪进度的控件标准与扩展 SPEC。
- [x] 确认下一阶段内置默认风格方向。
- [x] 确认控件状态、事件、焦点、主题、生命周期的统一标准契约。
- [x] 完成 Overlay Policy 统一设计和实现。
- [x] 完成旧控件第一轮标准化维护。
- [x] 完成第一批新增控件。
- [x] 完成第一批新增控件 XSON 化。
- [x] 完成工具型 UI 数据控件。
- [x] 完成控件示例、验证和文档同步。

## 阶段 0：Widget V2 基础层前置

- [x] 完成 `XUI Widget V2基础SPEC.md` 阶段 A-L。说明：阶段 A-L 的细项和总体验收均已完成；控件初始化中的 role、focus/tab、IME 默认策略已收敛到内部 widget role policy helper。
- [x] 按 Widget role 重新分类所有控件：Control、Container、Viewport、Overlay。说明：Control、Container、Viewport、Overlay role 已进入控件初始化和 XSON type 映射回归。
- [x] 所有控件改用 Widget V2 box model、clip、PaintContext、layer/zIndex/treeOrder。说明：公开控件已按阶段 L 重验；文本/图片/子树裁剪、overlay、debug layer 和 SplitLayout 等组合布局均有回归。
- [x] 所有控件改用 Widget V2 event route、focus、tab order、pointer capture、IME 策略。说明：输入、选择、拖拽、overlay、菜单、滚动、数据视图和基础应用控件均已覆盖事件路由、焦点/capture 和 IME 策略。
- [x] ScrollView、VirtualList、ListView、TreeView、TableView、PropertyGrid 改用 ScrollViewBase / VirtualScrollViewBase。说明：ScrollView 已直接复用 ScrollViewBase；VirtualList 已直接复用 VirtualScrollViewBase；ListView、TreeView、TableView、PropertyGrid 已内嵌 `tBase` 复用 VirtualScrollViewBase 的滚动、selected/hover/focus index、滚动条和 capture 状态。
- [x] DatePicker 作为基础应用控件补齐。说明：已支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 lab 验证。
- [x] ColorPicker 按功能完整基础控件重验并补齐。说明：已补齐 RGBA/hex 字段键盘编辑、提交/取消、错误态和编辑态 IME 策略。
- [x] RichTextView、CodeEditor、NodeGraph、Timeline 标记为可选高级组件，不阻塞核心基础层。说明：核心基础层只为它们提供 ScrollViewBase、VirtualScrollViewBase、文本输入、选择、焦点、clip 和绘制基础设施，不承诺第一阶段内置完整高级控件。

## 阶段 A：控件标准契约

- [x] 明确通用状态：normal、hover、active、focus、disabled。
- [x] 评估扩展状态是否进入公共 `XGE_XUI_STATE_*` bit：checked、selected、open、readonly、error、loading、dirty。
- [x] 明确状态优先级：disabled > active > hover > focus > checked/selected > normal。
- [x] 定义语义状态叠加规则：error/loading/dirty 不覆盖交互状态。
- [x] 统一事件命名：Click、Change、Changing、Select、Submit、Cancel、Open、Close。
- [x] 定义拖拽类控件 `Changing` 与 `Change` 的触发时机。
- [x] 定义选择类控件 `Select` 的触发时机。
- [x] 定义输入类控件 `Submit` 和 `Cancel` 的触发时机。
- [x] 明确所有 focusable 控件的 Enter、Space、Escape 行为。
- [x] 明确只读控件与禁用控件的焦点、选择、复制、输入差异。
- [x] 定义控件 setter 的 dirty layout / dirty paint 标记规则。
- [x] 定义 `Init/Unit` 生命周期规则：资源拥有权、widget 回调清理、NULL 安全。
- [x] 增加控件标准契约文档片段到公开文档。

## 阶段 B：内置风格与主题系统

目标是新增一套更精美的 XUI 默认风格。第一版倾向淡蓝色科技风、扁平、线框与色块，不依赖图片素材。

- [x] 确认默认风格名称：第一版作为 `xgeXuiThemeDefault` 的 Tech Blue 默认风格落地，后续可拆出命名 theme API。
- [x] 设计浅色科技风 token：背景、面板、描边、主文字、次文字、弱文字、accent、danger、warning、success。
- [x] 设计控件尺寸 token：control height、small height、large height、radius、padding、gap、border width。
- [x] 确认默认 UI 字体策略：中文 Windows 优先宋体 9pt，其他平台 fallback 到清晰小字号系统 UI 字体。
- [x] XUI 初始化时尝试加载默认 UI 字体：Windows 优先 `simsun.ttc`，9pt 按 12px 载入，失败时 fallback 到其他清晰小字号字体。
- [x] 定义字体 token：`font.ui.small`、`font.ui.body`、`font.ui.mono`。
- [x] 按 9pt 默认字体校准 Button/Input/List row/StatusBar 的高度、padding 和 baseline。
- [x] 设计状态色 token：normal、hover、active、focus、disabled、selected、checked、error。
- [x] 设计 focus ring 规则：颜色、宽度、内外描边、禁用态行为。
- [x] 设计输入控件 error 状态：边框、背景、提示文本、图标预留。
- [x] 设计 overlay 阴影替代方案：不依赖模糊图片，使用半透明边框和层级色块。
- [x] 设计 Button、IconButton、Input、CheckBox、Radio、Switch、Slider、Tabs 的标准视觉。
- [x] 确认 A 方案作为 Tech Blue 基底配色和整体控件样式。
- [x] 合并 D 方案的紧凑 Switch 尺寸和形态。
- [x] 合并 C 方案的 CheckBox / Radio 图形语言。
- [x] 强化 Panel / Window / Dialog 外边框层级，使容器外轮廓强于用户区内部边界和标题栏分隔线。
- [x] 定义边框层级：容器外边框 > focus ring > 控件边框 > 标题栏分隔线 > 表格内部网格线。
- [x] 设计 ListView、TreeView、TableView、PropertyGrid 的高密度工具界面视觉。
- [x] 设计 Dialog、Menu、ComboBox、Tooltip、Toast 的浮层视觉。
- [x] 设计内置小图标 bitmap mask：check、radio ring、radio dot、switch knob、triangle up/down/left/right、chevron up/down、close。
- [x] 定义内置 bitmap 图标的 tint 规则。
- [x] 定义没有 texture host 时的几何绘制 fallback。
- [x] 在 `xgeXuiThemeDefault` 或新增 theme API 中接入新 token。
- [x] 增加主题 lab 示例，展示所有控件状态。
- [x] 增加 Windows 人工验证记录或截图记录。
- [x] 确认旧示例在新默认风格下布局不破坏。

## 阶段 C：Overlay Policy

Overlay Policy 供 Popup、Menu、ComboBox、Dialog、MessageBox、Toast 共享。Tooltip 从公开控件调整为 widget 内部提示机制，只复用 overlay root、layer/zIndex/treeOrder 和 screen clamp。

- [x] 定义 overlay owner widget。
- [x] 定义 overlay popup widget。
- [x] 定义 open flag、modal flag、auto close outside、auto close escape。
- [x] 定义 placement：bottom-left、bottom-right、top-left、top-right、right-top、left-top、center、cursor、manual。
- [x] 定义 offset 和 anchor rect。
- [x] 定义 screen clamp 规则。
- [x] 定义 OverlayManager attach/detach、owner、bring-to-front 和 top overlay 查询；默认叠放使用 layer/zIndex/treeOrder，手动同层排序统一使用 widget zIndex。
- [x] 定义 focus restore widget。
- [x] 定义 owner hidden/disabled 时自动关闭。
- [x] 定义 modal backdrop 和 outside click 关闭顺序。
- [x] 实现 Popup 接入 overlay policy。
- [x] 实现 Menu 接入 overlay policy。
- [x] 实现 ComboBox 接入 overlay policy。
- [x] 实现 Tooltip 作为 widget 属性和 context 内部 overlay 服务，支持静态描述、NULL 清空和动态 resolver。
- [x] 实现 Dialog 接入 overlay policy。
- [x] 增加 `xui_overlay_policy_lab`。
- [x] 增加 Escape 只关闭 top overlay 的验证。
- [x] 增加关闭后 focus restore 验证。

## 阶段 D：旧控件维护：按钮与选择类

- [x] Button 增加 checked 可选状态。
- [x] Button 增加 loading 状态。
- [x] Button 增加 primary、danger 语义样式。
- [x] Button 支持 icon + text 布局。
- [x] Button 统一 Mouse、Touch、Enter、Space 触发 Click。
- [x] IconButton 与 Button 共享状态色和交互策略。
- [x] Toggle 对齐 Button/CheckBox 状态与事件命名。
- [x] CheckBox 补齐 keyboard toggle。
- [x] CheckBox 使用内置 `check_12` alpha bitmap 或等效几何 fallback 绘制对勾。
- [x] RadioGroup 补齐 keyboard navigation。
- [x] Radio 使用 C 方案空心圆和实心内点样式。
- [x] Switch 补齐 keyboard toggle 和 disabled 绘制。
- [x] Switch 改为 D 方案紧凑比例，避免轨道和 knob 过大。
- [x] 增加 `xui_button_standard_lab`。
- [x] 增加 `xui_choice_standard_lab`。

## 阶段 E：旧控件维护：输入与文本编辑

- [x] Input 增加 Change 回调。
- [x] Input 增加 Submit 回调。
- [x] Input 增加 Validate 或过滤策略。
- [x] Input 增加 MaxLength。
- [x] Input 增加 error 状态和错误文本。
- [x] Input 增加 clear button。
- [x] Input 增加 prefix/suffix icon 预留。
- [x] Input 明确 readonly 下可选择、可复制、不可修改。
- [x] Input 明确 password 模式复制策略。
- [x] Input 统一 IME candidate rect。
- [x] TextEdit 完善 undo/redo merge policy。
- [x] TextEdit 支持 readonly selection。
- [x] TextEdit 支持 find highlight 预留。
- [x] TextEdit 支持 line number 预留。
- [x] TextEdit 明确 Tab 输入策略。
- [x] 增加 `xui_input_standard_lab`。
- [x] 增加 `xui_text_edit_standard_lab`。

## 阶段 F：旧控件维护：列表、滚动、标签页

- [x] ListView 增加 selection mode：single、multi、range。
- [x] ListView 增加 disabled item。
- [x] ListView 增加 keyboard navigation：Arrow、Home、End、PageUp、PageDown。
- [x] ListView 增加 ensure visible。
- [x] ListView 增加 item renderer 预留。
- [x] VirtualList 增加 ensure index visible。
- [x] VirtualList 文档化 slot reuse 生命周期。
- [x] VirtualList 保持 selected item persistent。
- [x] VirtualList 预留 variable item height API。
- [x] ScrollView 增加 auto hide scrollbar 策略。
- [x] ScrollView 增加 ensure child visible。
- [x] ScrollView 明确 nested scroll policy。
- [x] ScrollView 支持 horizontal wheel policy。
- [x] ScrollBar 增加上下按钮三角标和点击翻页。
- [x] Tabs 增加 disabled tab。
- [x] Tabs 增加 close button。
- [x] Tabs 增加 dirty mark。
- [x] Tabs 增加 scrollable tab strip。
- [x] Tabs 增加 icon 预留。
- [x] 增加 `xui_list_standard_lab`。
- [x] 增加 `xui_scroll_standard_lab`。
- [x] 增加 `xui_tabs_standard_lab`。

## 阶段 G：第一批新增控件

第一批新增控件优先补齐高频应用 UI，不依赖复杂数据 adapter。

### G1：NumericInput

- [x] 定义 `xge_xui_numeric_input_t`。
- [x] 定义 `xgeXuiNumericInputInit/Unit`。
- [x] 支持 integer/float mode。
- [x] 支持 min/max。
- [x] 支持 step。
- [x] 支持 formatter。
- [x] 支持 keyboard Up/Down 调整。
- [x] 支持 spinner buttons 可选显示。
- [x] spinner buttons 使用内置 `triangle_up_8` / `triangle_down_8` bitmap 或等效几何 fallback。
- [x] 支持非法输入 error 状态。
- [x] 支持 Enter/blur 提交。
- [x] 增加 `xui_numeric_input_lab`。

### G2：MessageBox

- [x] 定义 `xge_xui_message_box_t`。
- [x] 支持 info、warning、error、question 类型。
- [x] 支持 OK、OKCancel、YesNo、YesNoCancel。
- [x] 支持 default button。
- [x] 支持 cancel button。
- [x] 支持 modal。
- [x] 支持 Enter 触发 default button。
- [x] 支持 Escape 触发 cancel button。
- [x] 保证 result callback 只触发一次。
- [x] 增加 `xui_message_box_lab`。

### G3：Toolbar

- [x] 定义 `xge_xui_toolbar_t`。
- [x] 支持 button item。
- [x] 支持 toggle item。
- [x] 支持 separator item。
- [x] 支持 group。
- [x] 支持 horizontal/vertical。
- [x] 支持 tooltip。
- [x] 支持 overflow menu。
- [x] 支持 item enabled/checked。
- [x] 增加 `xui_toolbar_lab`。

### G4：StatusBar

- [x] 定义 `xge_xui_status_bar_t`。
- [x] 支持 left/center/right sections。
- [x] 支持 text item。
- [x] 支持 progress item。
- [x] 支持 clickable item。
- [x] 支持 flexible spacer。
- [x] 支持窄宽度裁剪或省略策略。
- [x] 增加 `xui_status_bar_lab`。

## 阶段 H：数据控件

数据控件必须复用 VirtualList、ScrollView、Input、ComboBox 等基础能力。

### H1：TreeView

- [x] 定义 `xge_xui_tree_view_t`。
- [x] 定义 stable node id。
- [x] 定义 adapter 第一版 API。
- [x] 基于 VirtualList 映射可见行。
- [x] 支持 expand/collapse。
- [x] 支持 selected node。
- [x] 支持 icon 预留。
- [x] 支持 checkbox 预留。
- [x] 支持 ArrowLeft/ArrowRight 展开折叠。
- [x] 支持 ArrowUp/ArrowDown 移动选择。
- [x] 保证大树只创建可见行。
- [x] 增加 `xui_tree_view_lab`。

### H2：TableView

- [x] 定义 `xge_xui_table_view_t`。
- [x] 定义 column descriptor：id、title、width、minWidth、alignment。
- [x] 定义 row adapter。
- [x] 支持 fixed header。
- [x] 支持 fixed row height。
- [x] 支持 virtualized rows。
- [x] 支持 row selection。
- [x] 支持 column resize。
- [x] 支持 sort callback。
- [x] 支持 cell renderer。
- [x] 增加 `xui_table_view_lab`。

### H3：PropertyGrid

- [x] 定义 `xge_xui_property_grid_t`。
- [x] 支持 category。
- [x] 支持 property row。
- [x] 支持 text editor 类型声明和值展示。
- [x] 支持 number editor 类型声明和值展示。
- [x] 支持 bool editor 类型声明和值展示。
- [x] 支持 enum editor 类型声明和值展示。
- [x] 支持 color editor 预留和标记展示。
- [x] 支持 readonly property。
- [x] 支持 reset/default indicator。
- [x] 支持 validation error。
- [x] 增加 `xui_property_grid_lab`。

说明：H3 第一版已完成属性网格的数据结构、分类折叠、选择、视觉状态和 editor 类型承载；真正的内联编辑控件复用、提交/取消策略和 value change 事件留到后续体验补齐阶段。

## 阶段 I：体验补齐控件

- [x] ColorPicker 支持 swatch、RGBA numeric fields、hex input、palette。说明：RGBA/hex 字段支持键盘编辑，Enter 提交、Escape 取消，无效输入显示错误态。
- [x] Breadcrumb 支持 segment、separator、segment click、overflow collapse。
- [x] Accordion 支持 section、expanded state、single/multiple expand mode。
- [x] Toast 支持 queue、type、duration、close button、placement。
- [x] SearchBox 基于 Input 增加 search icon、clear、submit、suggestion 预留。
- [x] 增加对应 lab 和 case 文档。

## 阶段 J：XSON 控件扩展

### J1：第一批 XSON 控件

- [x] XSON 支持 `checkbox`。
- [x] XSON 支持 `radio`。
- [x] XSON 支持 `switch`。
- [x] XSON 支持 `slider`。
- [x] XSON 支持 `progress`。
- [x] XSON 支持 `tabs`。
- [x] 支持字段：checked、value、min、max、step、items、selected、onChange、onSelect。
- [x] 未注册事件名报错并包含字段路径。
- [x] 加载失败完整回滚。

### J2：第二批 XSON 控件

- [x] XSON 支持 `comboBox`。
- [x] XSON 支持 `menu`。
- [x] XSON 支持 `popup`。
- [x] XSON 支持 `tooltip` 作为任意 widget 的通用属性。
- [x] XSON 支持 `dialog`。
- [x] XSON 支持 `messageBox`。
- [x] XSON `popup`、`dialog`、`messageBox` 声明节点通过 overlay portal 挂到 overlay root；Menu/Tooltip 的实际浮层分别通过内部 Popup/context tooltip service 接入 overlay root。
- [x] XSON 支持 `numericInput`。
- [x] `comboBox` 支持字段：font、items、selected/value、dropDownHeight、color/background、hoverColor、activeColor、focusColor、disabledColor、textColor、popupColor、onSelect/onChange 未接入时报错。
- [x] `popup` 支持字段：owner、open、closeOnOutside、closeOnEscape、backgroundColor/background/color、onClose 未接入时报错。
- [x] widget `tooltip` 支持字符串简写和对象配置；对象字段：text、anchor、offsetX、offsetY、delay、followCursor、enabled。
- [x] 独立 `type:"tooltip"` 仅作为绑定到指定 owner 的兼容节点；支持字段：owner、text、anchor、offsetX、offsetY、delay、followCursor、enabled。
- [x] `tooltip` 不再支持 font、open、backgroundColor/background/color、textColor、onOpen、onClose；这些属于已删除的旧公开控件语义。
- [x] `menu` 支持字段：owner、font、items、enabledItems、menuWidth/width、maxHeight、itemHeight、open、x、y、backgroundColor/background/color、rowColor、selectedColor、textColor、disabledTextColor、onSelect 未接入时报错。
- [x] `dialog` 支持字段：font、title、open、modal、closeOnEscape、showClose、backdropColor、backgroundColor/background/color、titleColor、closeColor、onClose 未接入时报错。
- [x] `messageBox` 支持字段：font、title、message/text、kind/messageType/typeName、buttons、open、backdropColor、backgroundColor/background、titleColor、closeColor、messageColor/textColor、buttonColor、buttonHoverColor、buttonTextColor、onResult/onClose 未接入时报错。

### J3：第三批 XSON 控件

- [x] XSON 支持 `toolbar`。
- [x] `toolbar` 支持字段：font、items、orientation、itemWidth、itemHeight、separatorSize、backgroundColor/background、itemColor/color、hoverColor、activeColor、checkedColor、focusColor、disabledColor、separatorColor、textColor、disabledTextColor、onSelect 未接入时报错。
- [x] XSON 支持 `statusBar`。
- [x] `statusBar` 支持字段：font、items、barHeight/statusHeight、itemGap/gap、itemPadding、backgroundColor/background、borderColor、itemColor/color、hoverColor、activeColor、textColor、disabledTextColor、progressTrackColor、progressFillColor、onSelect 未接入时报错。
- [x] XSON 支持 `treeView`。
- [x] XSON 支持 `tableView`。
- [x] `tableView` 支持字段：font、columns、rows、selected、scrollY、headerHeight、rowHeight、backgroundColor/background、headerColor、headerTextColor、rowColor/color、selectedColor、gridColor、textColor、barColor、thumbColor、onSelect/onSort 未接入时报错。
- [x] XSON 支持 `propertyGrid`。
- [x] `propertyGrid` 支持字段：font、categories/properties、selected、rowHeight、nameWidth、backgroundColor/background、categoryColor、rowColor/color、selectedColor、gridColor、textColor、valueColor、readonlyColor、changedColor、errorColor、onSelect 未接入时报错。
- [x] XSON 支持 `breadcrumb`。
- [x] XSON 支持 `accordion`。
- [x] `accordion` 支持字段：font、sections、mode、selected、headerHeight、spacing、contentPadding、backgroundColor/background、headerColor/color、expandedColor、contentColor、borderColor、textColor、contentTextColor、onSelect 未接入时报错。
- [x] XSON 支持 `toast`。
- [x] `toast` 支持字段：font、items/toasts、placement、toastWidth、toastHeight、spacing、backgroundColor/background、borderColor、textColor、mutedTextColor、infoColor、successColor、warningColor、errorColor、closeColor、closeHoverColor、onClose 未接入时报错。

## 阶段 K：示例、验证与文档

- [x] 新增 `xui_standard_controls_lab`。
- [x] 新增 `xui_overlay_policy_lab`。
- [x] 新增 `xui_data_controls_lab`。
- [x] 新增 `xui_xson_controls_lab`。
- [x] 每个新增控件增加 `docs/case/xui-<control>-lab.md`。
- [x] 每个复杂交互控件增加 `docs/case/xui-<control>-policy-lab.md`。
- [x] 更新 `build_examples_all.bat --xui`。
- [x] 更新 `run_examples_smoke.bat`。
- [x] 更新 `docs/api/xui.md`。
- [x] 更新 `docs/case/xui-controls.md`。
- [x] 更新 Windows 人工验证记录。

## 验收矩阵

每个控件验收时至少检查：

| 项目 | 要求 |
| --- | --- |
| 初始化 | Init/Unit 可重复执行，无崩溃、无泄露 |
| 绘制 | normal/hover/active/focus/disabled 可区分 |
| 布局 | resize 后 rect、content rect 正确 |
| 输入 | mouse/touch/keyboard 行为符合文档 |
| 焦点 | focus in/out、capture lost/cancel 行为正确 |
| 主题 | theme/token 变更后可刷新 |
| Dirty | setter 标记 layout/paint 合理 |
| XSON | 支持类型字段校验和失败回滚 |
| 示例 | lab 可编译运行 |
| 文档 | API 和 case 文档同步 |

## 阻塞与决策记录

- [x] 决策：新内置风格第一版替换 `xgeXuiThemeDefault` 为 Tech Blue；后续如需要多主题，再新增命名 theme API。
- [x] 决策：扩展状态第一版先保留在控件私有字段或控件专用 item 状态中，暂不新增公共 bit。
- [x] 决策：复杂控件事件第一版沿用现有 `Click/Change/Select/Submit/Open/Close` 回调，不引入统一 value change 事件结构。
- [x] 决策：TableView 第一版不支持列冻结，优先完成虚拟行、列宽、选择、排序和 cell renderer。
- [x] 决策：PropertyGrid 第一版不支持嵌套对象，优先完成分类、属性行、editor 类型承载、readonly/default/error 状态。
- [x] 决策：ColorPicker 进入核心 XUI 控件库，并纳入 lab、case 文档和 XSON 聚合验证。
- [x] 决策：XSON 未支持字段和未注册事件名采用严格报错，并在错误中包含字段路径。
- [x] 决策：RichTextView、CodeEditor、NodeGraph、Timeline 作为可选高级组件路线推进，不阻塞 Widget 基础层和基础应用控件验收。
