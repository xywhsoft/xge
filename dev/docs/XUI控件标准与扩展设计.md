# XUI 控件标准与扩展设计

本文档定义 XUI 下一阶段控件体系升级方向。目标是在保持 XUI 轻量、retained、可手写 C API 的前提下，把现有控件打磨成行为一致的标准件，并补齐构建工具型应用、编辑器 UI 和游戏内工具 UI 所需的高价值控件。

本文档是设计文档，描述方向、分层和取舍；具体 API、行为和验收条件见 `XUI控件标准与扩展SPEC.md`。

## 1. 背景

当前 XUI 已经具备 widget tree、布局、事件、焦点、捕获、主题、Page Loader、XSON、基础控件和浮层能力。已有控件覆盖了按钮、文本、输入、选择、列表、滚动、弹窗、菜单、标签页、窗口等常见场景。

下一阶段的问题不再是“是否有控件”，而是：

- 同类控件的状态、事件、键盘交互和主题字段是否一致。
- 控件是否能支撑实际工具应用，而不是只支撑示例。
- 复杂控件是否复用已有 ListView、VirtualList、Popup、Input 等能力。
- XSON 声明式层是否逐步覆盖 C API 已有控件。
- 控件测试是否能覆盖输入策略、焦点策略和边界状态。

## 2. 设计目标

- 建立统一控件标准契约，让每个控件都能按同一套状态、事件、样式、生命周期和测试标准实现。
- 优先提升现有控件质量，再增量加入高价值控件。
- 新增控件优先服务工具型 UI：资源树、属性面板、表格、工具栏、状态栏、消息框。
- 复杂控件必须复用现有基础设施，避免每个控件重复实现滚动、弹出层、选择、文本输入和主题逻辑。
- C API 作为权威能力面，XSON 按阶段开放声明式创建能力。
- 默认视觉保持克制、清晰和可扫描，不把业务页面样式固化进控件。
- 每个控件都必须有可运行示例和策略验证用例。

## 3. 非目标

- 不实现浏览器级控件体系。
- 不实现完整 CSS、DOM、JS 事件模型。
- 不实现大型 IDE Docking/Ribbon 框架。
- 不把所有业务界面都做成核心控件。
- 不为复杂数据控件创建大量长期存活 widget。
- 不在控件内部直接绑定业务数据源格式。
- 不要求第一阶段所有控件都进入 XSON。

## 4. 控件标准契约

每个 XUI 控件都应遵守同一套标准契约。契约不是要求每个控件都支持所有状态和事件，而是要求支持项命名一致、行为一致、文档一致。

### 4.1 状态模型

通用状态：

- `normal`
- `hover`
- `active`
- `focus`
- `disabled`

扩展状态：

- `checked`：Toggle、CheckBox、Radio、Switch、可切换 Button。
- `selected`：ListView、VirtualList、Tabs、TreeView、TableView。
- `open`：ComboBox、Popup、Menu、Tooltip、Dialog。
- `readonly`：Input、TextEdit、PropertyGrid 单元格。
- `error`：Input、NumericInput、PropertyGrid 编辑器。
- `loading`：Button、Progress、异步数据控件。
- `dirty`：Tabs、TreeView、PropertyGrid 节点可选扩展。

状态优先级建议：

```text
disabled > active > hover > focus > checked/selected > normal
```

`error`、`loading`、`dirty` 是语义标记，不应覆盖交互状态，而应叠加影响颜色、图标或装饰。

### 4.2 事件模型

事件命名应按语义统一：

- `Click`：离散点击确认。
- `Change`：值已提交变化。
- `Changing`：拖拽或编辑过程中的临时变化。
- `Select`：选择项变化。
- `Submit`：输入提交或默认动作。
- `Cancel`：取消编辑、关闭或回退。
- `Open` / `Close`：浮层或容器展开状态变化。

建议保留当前函数指针风格，逐步增加专用回调类型；复杂控件可以使用通用事件结构，但不应把所有控件都退化成字符串事件。

### 4.3 输入模型

所有可交互控件必须明确支持：

- 鼠标点击、移动、按下、释放。
- 触摸按下、移动、释放。
- 键盘焦点、Enter、Space、Escape、Tab。
- 必要时支持方向键、Home、End、PageUp、PageDown。
- 滚动类控件支持鼠标滚轮和触摸拖动策略。

控件必须在不可见或禁用时停止处理交互事件。只读控件可以获得焦点和选择，但不能修改值。

### 4.4 焦点模型

XUI 应维持单一焦点 widget。控件需要明确：

- 是否默认 focusable。
- 鼠标点击是否获取焦点。
- Tab 顺序是否参与。
- Escape 是否取消状态。
- Enter 是否触发默认动作。
- 关闭 Popup/Dialog 后焦点是否恢复到 owner。

浮层、菜单和模态对话框必须遵守焦点恢复规则，避免关闭后焦点丢失。

### 4.5 样式模型

控件颜色和尺寸应来自三层来源：

1. 控件默认主题。
2. Context theme/token。
3. 控件本地 setter 或 XSON inline 字段。

控件新增字段优先使用语义 token，而不是硬编码颜色。例如：

- `control.background`
- `control.hover`
- `control.active`
- `control.focus`
- `control.disabled`
- `text.primary`
- `text.disabled`
- `accent`
- `danger`
- `warning`
- `success`

控件尺寸同样应优先使用 token：

- `control.height`
- `control.paddingX`
- `control.paddingY`
- `control.gap`
- `control.radius`
- `font.body`
- `font.mono`

### 4.6 生命周期模型

所有控件应遵守：

- `Init` 绑定 widget、安装默认回调、设置默认状态。
- `Unit` 清理控件持有的资源，并只在 widget 仍由该控件占用时清理回调。
- 控件不释放外部传入的 font、texture、用户数据，除非 API 明确声明拥有权。
- Page Loader 创建的控件由 page arena 统一释放。
- 控件变更布局相关字段时标记 layout dirty。
- 控件变更视觉字段时标记 paint dirty。

### 4.7 内置 Tech Blue 风格决策

下一阶段内置风格以 `xui-style-a-ice-console.svg` 为基底，合并 `xui-style-d-cyan-workbench.svg` 的紧凑 Switch 设计，并采用 `xui-style-c-aqua-minimal.svg` 中 CheckBox / Radio 的图形语言。

视觉方向：

- 淡蓝色科技风。
- 扁平，不使用图片背景、渐变大面积装饰或复杂阴影。
- 主要由线框、色块、轻量描边和语义色组成。
- 默认控件边缘比用户内容区内部边界更明确，避免标题栏、面板边界和内容区边界层级混乱。
- Panel / Window / Dialog 的外边框必须比内部标题栏分隔线更突出。
- 标题栏可以使用浅色填充和细分隔线，但不应强于容器外轮廓。
- 高密度工具 UI 优先保证可扫描性，控件半径保持克制。

建议边框层级：

```text
容器外边框 > focus ring > 控件边框 > 标题栏分隔线 > 表格内部网格线
```

建议形态：

- Button：A 方案的中等圆角矩形、清晰描边、accent 实心主按钮。
- Input：白底、淡蓝描边、focus 时高亮描边和细 focus ring。
- Switch：D 方案的紧凑比例，轨道高度接近普通输入框的 70% 到 80%，避免 A 方案过大。
- CheckBox：C 方案的圆角方框和清晰对勾。
- Radio：C 方案的空心圆加实心内点。
- Slider：A 方案的细轨道、accent fill、白底描边 knob。
- Panel：A 方案配色，但增强外边框 alpha 或宽度，让容器轮廓明确。

默认字体：

- 默认 UI 字号以 9pt 为基准，优先保证小字号边缘清晰和高密度界面可读性。
- 中文 Windows 环境第一候选为宋体 9pt，适合小字号下清晰显示。
- 如果平台或字体加载路径不适合宋体，可 fallback 到平台清晰小字号 UI 字体，例如 Microsoft YaHei UI、Segoe UI、Arial 或系统默认 sans-serif。
- 字体选择应进入 theme/token，而不是写死在控件内部。
- 控件高度、padding 和 baseline 应围绕 9pt 默认字体重新校准，避免文字在 Button/Input/List row 中上下偏移。
- 后续可提供 `font.ui.small`、`font.ui.body`、`font.ui.mono` 三类 token。

### 4.8 内置 bitmap 图标资源

部分控件需要小尺寸符号，例如 CheckBox 对勾、ComboBox 下拉箭头、ScrollBar 上下按钮三角、NumericInput 上下 stepper 三角。为了保证风格统一，并避免依赖外部图片资源，XUI 可以在代码中构建内置 bitmap 数据。

建议方案：

- 使用小尺寸单通道 alpha bitmap 或 1-bit mask 数据。
- 运行时按 theme 颜色 tint，避免为不同状态维护多套图片。
- 内置资源可以合并为一个小型 icon atlas，也可以按图标懒创建 texture。
- 原始数据以 C 数组保存，位于 XUI 内部实现文件，不进入资源系统。
- 支持 DIP scale 时优先选择 1x / 2x 两套 alpha mask，或使用最近邻放大后再由线性采样显示。
- 图标只用于小符号，不用于大面积 UI 背景。
- 如果 host 不支持纹理路径，控件必须能回退到几何绘制。

第一批内置 bitmap 图标：

- `check_12`：CheckBox 对勾。
- `radio_dot_8`：Radio 选中内点，必要时也可几何绘制。
- `chevron_down_8`：ComboBox、Menu 子项。
- `triangle_up_8`：ScrollBar 上按钮、NumericInput step up。
- `triangle_down_8`：ScrollBar 下按钮、NumericInput step down。
- `close_8`：Input clear、Tab close、Dialog close。

## 5. 分层设计

### 5.1 原子控件层

原子控件直接绑定一个 widget，负责绘制和事件：

- Button
- IconButton
- Label
- Image
- Input
- TextEdit
- Toggle
- CheckBox
- Radio
- Switch
- Slider
- Progress
- Separator
- ScrollBar

这层控件必须简单、稳定、可独立测试。

### 5.2 容器控件层

容器控件管理子 widget 或内容区域：

- Panel
- ScrollView
- ListView
- VirtualList
- Tabs
- SplitLayout
- Window

这层控件重点是布局、滚动、选择、子元素生命周期和性能。

### 5.3 浮层控件层

浮层控件使用 overlay root：

- Popup
- Menu
- ComboBox
- Dialog
- MessageBox
- Toast

这层应共享统一 overlay policy：owner、placement、z-order、auto close、Escape、outside click、focus restore、modal stack。

Tooltip 不再属于这一层的公开控件。它是 XUI 内部机制：每个 widget 都带有 tooltip 描述，context 负责统一命中检测、延迟、定位、绘制和关闭。

### 5.3.1 Tooltip 机制

- 每个 `xge_xui_widget_t` 都具备 `tTooltip` 属性。
- 普通文本提示使用 `xgeXuiWidgetSetTooltipText(widget, text)`。
- 自定义提示使用 `xgeXuiWidgetSetTooltip(widget, desc)`，由 `measure/paint` 回调决定内容尺寸和绘制。
- `xgeXuiWidgetClearTooltip(widget)` 清除提示。
- Tooltip 支持相对目标元素底部、顶部、左侧、右侧，或相对鼠标光标定位。
- Tooltip 支持 offset、delay、follow cursor。
- Tooltip popup 是 context 内部 overlay widget，不参与 hit test，不接管 focus，不改写 owner 的 event/capture 回调。
- Tooltip 打开状态通过 `xgeXuiTooltipIsOpen(context)`、`xgeXuiTooltipGetOwner(context)`、`xgeXuiTooltipGetRect(context)` 查询。

### 5.4 数据控件层

数据控件复用 VirtualList、ScrollView、Input、ComboBox 等基础能力：

- TreeView
- TableView
- PropertyGrid

这层不应假设业务数据结构。数据通过 adapter 或显式 item API 进入控件。

### 5.5 应用骨架控件层

应用骨架控件用于常见工具 UI，但保持轻量：

- Toolbar
- StatusBar
- Breadcrumb
- Accordion

这层可进入核心库，但必须由已有布局和原子控件组合实现，避免引入独立复杂布局系统。

## 6. 新增控件路线

### 6.1 第一优先级

第一优先级控件直接提升 XUI 构建实际工具应用的能力：

- `NumericInput`：数字输入、step、min/max、格式化。
- `Toolbar`：工具按钮、分组、overflow。
- `TreeView`：层级数据、展开折叠、选择。
- `TableView`：表头、列宽、排序、行选择、虚拟化。
- `PropertyGrid`：属性名/属性值编辑器。
- `MessageBox`：标准确认框。

### 6.2 第二优先级

第二优先级用于补齐常见应用体验：

- `StatusBar`
- `Breadcrumb`
- `Accordion`
- `Toast`
- `ColorPicker`
- `SearchBox`

### 6.3 第三优先级

第三优先级复杂度较高，适合在基础控件稳定后推进：

- `DatePicker`
- `TreeSelect`
- `MenuBar`
- `CodeEdit`
- `Timeline`
- `NodeGraph`

## 7. 现有控件优化方向

### 7.1 Button / IconButton

- 增加 checked、loading、danger、primary 语义。
- 支持 icon + text 排列。
- 支持默认按钮和取消按钮语义。
- 统一 Enter/Space 触发策略。
- 明确 disabled 时是否绘制 focus ring。

### 7.2 Input

- 增加 `Change`、`Submit`、`Validate`。
- 支持 max length、filter、error text、clear button、prefix/suffix icon。
- 明确 readonly 与 disabled 的焦点和选择行为。
- 统一剪贴板、上下文菜单、IME candidate rect。

### 7.3 TextEdit

- 强化 undo/redo 合并策略。
- 支持查找高亮、行号可选、tab 输入策略。
- 优化 word wrap、滚动定位、选择绘制。
- 支持只读模式下复制和选择。

### 7.4 ListView / VirtualList

- 增加多选、范围选择、键盘导航。
- 支持 item renderer 或 slot bind。
- 支持滚动到 index、保持选中项可见。
- VirtualList 支持可变高度作为后续扩展。

### 7.5 ScrollView / ScrollBar

- 统一嵌套滚动策略。
- 支持滚动到子 widget。
- 支持自动隐藏滚动条。
- 支持横向滚轮或 Shift+Wheel 横向滚动。

### 7.6 Popup / Menu / ComboBox / Dialog

- 抽出统一 overlay policy。
- 支持 placement、anchor rect、screen clamp。
- 支持 focus restore。
- 支持 modal stack 和 outside click 策略。
- ComboBox 支持 disabled item、filter、keyboard selection。
- Tooltip 仅复用 overlay root 和 screen clamp，不复用 Popup 控件对象，避免公开控件语义和内部提示机制混淆。

### 7.7 Tabs

- 支持关闭按钮、滚动 tab strip、disabled tab。
- 支持 dirty 标记和图标。
- 支持键盘切换。

### 7.8 Window

- 支持最小尺寸、bring-to-front、窗口管理器。
- 支持自定义 title bar 内容。
- 支持停靠/吸附作为后续扩展。

## 8. XSON 扩展策略

XSON 不应一次性开放所有控件。建议按稳定度分批：

第一批：

- checkbox
- radio
- switch
- slider
- progress
- tabs

第二批：

- comboBox
- menu
- popup
- tooltip
- dialog
- messageBox

第三批：

- toolbar
- treeView
- tableView
- propertyGrid
- numericInput

声明式控件必须满足：

- 字段类型严格校验。
- 未支持事件字段必须报错，而不是静默忽略。
- onClick/onChange/onSelect 等绑定只引用 C 侧 binder 名称，不支持脚本。
- loader 失败必须回滚已创建 widget 和控件资源。

XSON 中的 `tooltip` 优先作为任意 widget 的通用属性加载，不是控件实例。支持两种写法：`"tooltip":"说明文本"` 简写，以及 `"tooltip":{"text":"说明文本","anchor":"right","offsetX":4,"offsetY":2,"delay":0,"followCursor":false}` 对象配置。独立 `type:"tooltip"` 节点保留为绑定到指定 `owner` 的兼容写法；不再支持 `open/onOpen/onClose/font/backgroundColor/textColor` 这类旧公开控件字段。

## 9. 测试与示例策略

每个新增或升级控件至少包含：

- `*_lab`：人工查看和交互示例。
- `*_policy_lab`：输入、焦点、关闭、边界策略验证。
- `*_proc_lab`：事件回调和状态变更验证。
- 必要时增加 validation 示例，用于自动 smoke。

复杂控件至少覆盖：

- 空数据。
- 大数据。
- disabled/readonly。
- keyboard navigation。
- focus lost。
- resize/layout dirty。
- theme change。

## 10. 推荐实施顺序

1. 抽象 overlay policy，统一 Popup、Menu、ComboBox、Tooltip、Dialog。
2. 补齐 Input、ListView、VirtualList、Tabs 的标准交互。
3. 增加 NumericInput、MessageBox、Toolbar、StatusBar。
4. 基于 VirtualList 实现 TreeView。
5. 基于 VirtualList/ScrollView 实现 TableView。
6. 基于已有编辑器控件实现 PropertyGrid。
7. 分批把成熟控件接入 XSON。

## 11. 风险

- 过早引入 TableView、PropertyGrid 可能导致 API 固化过快。
- 复杂控件如果不复用 VirtualList，会造成性能和维护成本失控。
- XSON 过早支持复杂控件可能让 loader 膨胀。
- Overlay 规则如果继续分散在各控件里，ComboBox/Menu/Dialog 行为会长期不一致。
- 主题 token 如果没有语义分层，后续皮肤和状态扩展会困难。

## 12. 决策

- XUI 控件库继续保持小核心、可组合、可测试。
- 第一阶段以标准化现有控件为主，新增控件以 NumericInput、Toolbar、MessageBox 为起点。
- TreeView、TableView、PropertyGrid 是下一阶段工具 UI 能力的核心，但必须建立在 VirtualList、ScrollView、Input、ComboBox 稳定之后。
- XSON 只暴露已经稳定的控件能力，不把实验性复杂控件提前声明式化。
