# XUI 控件编辑管理层设计

本文档用于规划 UIDesign 工程中的 XUI 界面编辑器。目标是支持所有 XUI 控件的设计时编辑，并抽象出统一的控件编辑管理层，使新增 XUI 控件可以通过注册描述、属性 schema 和专用编辑器扩展接入，而不是把逻辑写死在画布或属性表里。

工程级决策见 `tools/UIDesign/工程规划.md`。工具标题统一为 `XUI Designer`，工程形态为纯 C 工具。

## 目标

- 所有 XUI 控件都能在设计器中创建、选择、移动、调整尺寸、编辑属性、预览运行态。
- 简单控件主要通过属性表编辑。
- 集合控件提供 item/column/node/category 等结构化管理。
- 容器控件提供子节点、slot、布局辅助线和拖放规则。
- 复杂复合控件提供专用设计面板，例如 DockPanel 的窗口、区域、停靠状态和布局状态管理。
- 新控件通过注册 `ControlEditorDescriptor` 接入，不修改画布核心流程。

## 基本判断

编辑器不应直接把运行时 `xge_xui_widget` 当作唯一源数据。推荐采用：

1. `UIDocument` 作为设计时唯一真源，保存 XSON AST、节点 id、属性、样式、模板、资源引用和控件扩展数据。
2. `PreviewRuntime` 根据 `UIDocument` 生成或局部刷新 XUI widget tree。
3. `ControlEditorRegistry` 根据节点 type 找到对应控件编辑器。
4. 属性表、结构面板、画布手柄和专用编辑面板都通过控件编辑器访问同一份 `UIDocument`。

这样可以避免从运行时 widget 反向还原 XSON，也便于撤销/重做、批量修改和格式化保存。

## 主界面布局风格

XUI Designer 的默认界面应采用接近 Visual Basic / Visual Studio Form Designer 的工作台布局，而不是网页式或卡片式 UI。核心观感是“工具箱 + 窗体设计器 + 工程/属性窗口”。

根布局直接采用 `菜单 -> 工具栏 -> DockPanel -> 状态栏`。DockPanel 是 UIDesign 本体的工作区基础框架，Toolbox、Structure、Form Designer、Project Explorer、Properties、Actions、Output 都是 DockWindow，不在 DockPanel 外再包一层三栏布局。

```text
+--------------------------------------------------------------------------------+
| Menu: File Edit View Project Format Run Tools Window Help                       |
+--------------------------------------------------------------------------------+
| Toolbar: New Open Save | Cut Copy Paste | Undo Redo | Start Preview | Align     |
+--------------------+---------------------------------------+-------------------+
| Toolbox            | Form Designer                         | Project Explorer  |
|                    |                                       | - ui/main.xson    |
| Pointer            |  +---------------------------------+  | - styles.xson     |
| Common Controls    |  | Form / Page Surface             |  | - resources       |
| - Label            |  |                                 |  +-------------------+
| - Button           |  |  [Button] [Input]               |  | Properties        |
| - TextBox/Input    |  |                                 |  | Object: Button1   |
| - CheckBox         |  |  selection handles              |  | (Name)            |
| - ComboBox         |  |  grid / snap lines              |  | Text              |
| Containers         |  |                                 |  | Width / Height    |
| - Panel            |  +---------------------------------+  | Layout            |
| - Tabs             |                                       | Events            |
| - DockLayout       |                                       | Actions (...)     |
+--------------------+---------------------------------------+-------------------+
| Document Tabs: Design | XSON | Preview                                        |
+--------------------------------------------------------------------------------+
| Immediate / Output / Validation / Event Log                                     |
+--------------------------------------------------------------------------------+
```

VB 风格对应关系：

- Toolbox：控件库，按 Common Controls、Containers、Data、Menus、Workbench 分组。
- Form Designer：中央所见即所得画布，显示页面/Form 边界、网格、吸附线、选择框和 resize handles。
- Project Explorer：工程树，显示 XSON 页面、样式、资源和生成物。
- Properties Window：属性表，默认按 Categorized/Alphabetic 两种视图切换。
- Events View：属性表可切到事件页，绑定 onClick、onChange、Command、HotKey。
- Immediate/Output：底部日志、校验错误、XSON trace、运行预览输出。
- Custom Editors：复杂控件编辑器以 VB 工具窗口方式 dock/floating，例如 DockLayout Designer、Menu Editor、Table Editor。

设计口径：

- 默认不是现代网页式大面板，而是紧凑、密集、工具化的桌面 IDE。
- 控件放置应支持双击 Toolbox 自动放到当前 Form，也支持拖放。
- 中央画布应提供 VB 类似网格、snap、align、make same size、tab order 等设计器命令。
- 属性窗口优先稳定、密集、可键盘操作，不追求大号卡片。
- 复杂控件的专用编辑器可以像 VB 的 Menu Editor 一样从属性按钮、右键菜单或工具栏打开。

## 运行模式与刷新策略

XUI Designer 本体是工具类应用，应使用 APP 模式 / 按需刷新策略，而不是固定帧率持续刷新。这里直接吸收 `tools/mapedit` 的经验：工具窗口大部分时间处于空闲状态，持续 `update/draw/present` 会无意义消耗 CPU。

启动参数建议：

```c
tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC | XGE_INIT_ON_DEMAND;
tDesc.iRunMode = XGE_RUN_GAME_LOOP;
tDesc.iTargetFPS = 30; /* 仅作为有连续刷新请求时的上限，不代表空闲持续刷新 */
```

如果运行自动化 smoke 或截图测试，可参考 mapedit 的 `--frames N` 模式：测试模式关闭 `XGE_INIT_ON_DEMAND`，避免没有后续刷新请求时测试无法退出。

### 刷新规则

- 启动完成后请求首帧。
- 输入、鼠标、触摸、窗口 resize、打开/关闭弹层、切换文档、切换选中控件、滚动、拖拽、属性修改、撤销/重做、校验结果变化都必须请求刷新。
- 没有刷新请求时，不应执行 `glClear`、不跑完整 scene update/draw、不 present。
- XUI 状态变化必须显式调用 `xgeXuiWidgetMarkPaint`、`xgeXuiWidgetMarkLayout` 或 `xgeXuiWidgetMarkLayout/MarkPaint` 的上层封装。
- 修改 `UIDocument` 后，由 `UIDesignInvalidateNode` / `UIDesignInvalidateCanvas` 统一标记预览树和属性窗口 dirty。
- 需要连续动画的场景必须显式进入临时连续刷新，例如 hover 动画、拖拽中的 dock indicator、popup 动画、预览运行态动画；动画结束后退出连续刷新。

这不是严格的“局部脏矩形重绘”。第一阶段目标是“空闲不重绘”。真正的矩形级局部重绘需要 scissor、缓存层、遮挡处理、浮动窗口重叠处理和外部 surface invalidation，后续再独立设计。

### UIDesign 刷新 API

建议在 UIDesign 内部封装刷新入口，避免业务代码直接散落调用底层 render request：

```c
void UIDesignInvalidate(ui_app_t* app, int reason);
void UIDesignInvalidateNode(ui_app_t* app, ui_node_id nodeId, int reason);
void UIDesignInvalidateInspector(ui_app_t* app);
void UIDesignInvalidateStructure(ui_app_t* app);
void UIDesignBeginContinuousRefresh(ui_app_t* app, int reason);
void UIDesignEndContinuousRefresh(ui_app_t* app, int reason);
```

实现策略：

- 普通 invalidate：标记相关模型 dirty，调用 XUI mark paint/layout，再请求一帧。
- 连续刷新：维护引用计数或 reason bitset，在 update 中每帧请求下一帧。
- 属性表编辑：每次提交值后只刷新受影响节点；拖动 numeric/slider 时可以临时连续刷新。
- 画布拖拽：拖动期间连续刷新，鼠标释放后退出连续刷新并合并为一次 undo command。
- 预览模式：若用户运行的是动画 UI，可开启连续刷新；退出预览后恢复按需刷新。

### 从 mapedit 继承的注意事项

- 只改数据不请求刷新是错误的。典型问题是 hover、selection、drag indicator 数据变了，但画面要等下一次外部事件才更新。
- 所有控件编辑器的 `handleCanvasAction`、`applyProperty`、item/node/column 管理器都必须返回明确的 invalidation reason。
- 日志、校验、输出窗口追加内容后也要请求刷新，否则底部面板不会及时更新。
- smoke 测试要有帧数上限和退出条件，不能依赖人工输入触发后续帧。

## 控件编辑管理层

### ControlEditorDescriptor

每种 XUI 控件注册一个描述：

```c
typedef struct ui_control_editor_desc_t {
    const char* type;                 /* XSON type，例如 "button" */
    const char* displayName;
    int category;                     /* basic/input/data/layout/overlay/workbench/service */
    int role;                         /* control/container/viewport/overlay/service */
    uint32_t capabilities;            /* canHaveChildren/canResize/canEditItems/canOpenOverlay/... */
    const ui_property_schema_t* props;
    int propCount;
    const ui_child_rule_t* childRules;
    int childRuleCount;
    const ui_design_action_t* actions;
    int actionCount;
    ui_control_editor_vtbl_t vtbl;
} ui_control_editor_desc_t;
```

### 统一能力位

- `PROPERTY_ONLY`：只需属性表即可完成主要编辑。
- `CHILDREN`：允许普通 children，可在结构树和画布拖放子控件。
- `SLOTS`：有命名 slot，例如 Window client、Accordion section client、DockWindow client。
- `ITEMS`：有非 widget item 数据，例如 ListView items、ComboBox options、Menu items。
- `COLUMNS_ROWS`：有表格列/行/单元格管理。
- `TREE_NODES`：有树节点管理。
- `OVERLAY_PREVIEW`：需要打开弹层预览和锚点编辑。
- `STATE_PREVIEW`：需要切换 hover/active/focus/disabled/open/selected 等状态预览。
- `DIRECT_MANIPULATION`：支持画布手柄直接调参，例如 SplitLayout divider、DockPanel 拖停靠。
- `SPECIAL_EDITOR`：需要专用面板，不应只依赖属性表。
- `SERVICE_ONLY`：不是普通页面节点，例如 Toast 服务。

### 控件编辑器虚表

```c
typedef struct ui_control_editor_vtbl_t {
    void (*buildDefaultNode)(ui_document_t* doc, ui_node_t* node);
    int  (*validate)(ui_document_t* doc, ui_node_t* node, ui_validation_t* out);
    void (*applyProperty)(ui_document_t* doc, ui_node_t* node, const char* prop, xvalue value);
    void (*buildStructure)(ui_document_t* doc, ui_node_t* node, ui_structure_model_t* out);
    void (*buildInspector)(ui_document_t* doc, ui_node_t* node, ui_property_grid_model_t* out);
    void (*buildAdorners)(ui_document_t* doc, ui_node_t* node, ui_canvas_adorners_t* out);
    int  (*handleCanvasAction)(ui_document_t* doc, ui_node_t* node, const ui_canvas_action_t* action);
    int  (*openSpecialEditor)(ui_document_t* doc, ui_node_t* node, const char* editorId);
} ui_control_editor_vtbl_t;
```

默认编辑器实现通用属性、布局属性、样式属性和基础画布手柄。复杂控件只覆盖需要的部分。

## 通用属性分层

所有普通节点都应有这些属性组：

- Identity：`id`、`name`、`type`、备注、是否导出。
- Layout：`width`、`height`、`min/max`、`margin`、`padding`、`dock`、`anchor`、`grow`、`alignX/Y`、`justify`、`grid`。
- Visual：`background`、`borderColor`、`borderWidth`、`radius`、`opacity`、`debugOutline`、状态样式。
- Interaction：`visible`、`enabled`、`focusable`、`tabStop`、`tabIndex`、`hitTestVisible`、`inputTransparent`、`tooltip`、`cursor`。
- Layering：`zIndex`、`layer`、`overflow`、`clip`。
- Events：`onClick`、语义事件、hotkey、command，全部只绑定 C 侧注册名或编辑器工程中的动作名，不支持脚本。
- Binding：`${key}` model binding、资源引用、token 引用、style 引用。

属性表只显示当前控件可用的属性；隐藏运行时只读字段。

## 控件逐项评估

### Widget / 基础节点

属性：通用属性全量、role、box model、状态样式、owner draw、tooltip、事件兴趣。

额外编辑：结构树节点、画布选择框、边距/内边距可视化、clip/overflow 预览、状态预览。

抽象：作为所有控件的基础 schema，不单独作为用户常用控件；可提供 `customWidget` 节点给高级用户。

### Layout 容器：panel / row / column / stack / grid / dock / absolute

属性：通用布局属性、layout 类型、gap、align、justify、grid columns/rowHeight/rowGap/columnGap、dock region、anchor。

额外编辑：拖放 children、布局辅助线、网格列编辑器、dock 区域 drop 提示、绝对布局的坐标手柄。

抽象：`ContainerEditor`，负责 child rule、drop target、reorder、slot rect 计算和布局 adorners。

### Label

属性：text、font、textColor、disabledColor、textAlign、textVAlign、underline、wrap/clip、通用属性。

额外编辑：画布内双击改文本；状态预览即可。

抽象：`TextControlEditor`。

### Button

属性：text、font、textColor、semantic、selectable、selected、icon、iconColor、iconPlacement、iconSize、gap、normal/hover/active/focus/disabled colors、badge、patch、onClick。

额外编辑：属性表为主；状态预览 hover/active/focus/disabled/selected；图标资源选择器；badge 位置预览。

抽象：`ActionControlEditor`，暴露 command/onClick 绑定。

### Image

属性：texture/source/srcRect/tint/color/mode/align/customRect、资源路径、通用属性。

额外编辑：资源选择器、srcRect 裁切编辑器、fit/fill/stretch 预览。

抽象：`ResourceControlEditor`。

### Separator

属性：orientation、thickness、color、background、通用尺寸。

额外编辑：属性表即可；画布中根据方向限制尺寸手柄。

抽象：简单属性控件。

### Input

属性：text/value、placeholder、font、textColor/background/focus/cursor、frame colors、disabled colors、password、readonly、disabled、maxLength、textAlign、selection、decorations、clear button、prefix/suffix icons、filter/change/submit。

额外编辑：输入态预览、密码态预览、装饰按钮管理、placeholder 快速编辑。

抽象：`ValueControlEditor`，支持 value binding、validation、submit action。

### NumericInput

属性：value、min/max、step、integer、precision、spinnerVisible、spinnerWidth、spinnerColors、formatter、change/error callbacks。

额外编辑：范围滑条预览、spinner 手柄或宽度编辑、数值校验。

抽象：`ValueControlEditor` + number validator。

### TextEdit

属性：text、font、readonly、wordWrap、lineNumbers、lineNumberWidth、currentLineColor、findHighlights、scrollbarMode、scrollX/Y、colors、frame colors、disabled colors。

额外编辑：多行文本弹窗编辑、代码/普通文本模式、行号预览、滚动位置预览。

抽象：`TextDocumentControlEditor`，支持大文本外部编辑器。

### CheckBox

属性：text、checked、font、textColor、boxColor、checkedColor、textures、indicatorSize、gap、change。

额外编辑：属性表为主；checked/unchecked 状态切换预览。

抽象：`ChoiceControlEditor`。

### Radio / RadioGroup

属性：text、checked、groupId、value、font、colors、textures、indicatorSize、gap、change。

额外编辑：RadioGroup 管理器，允许一组 radio 的 value、默认选中和互斥关系可视化。

抽象：`ChoiceGroupEditor`，group 不是普通画布控件时作为设计时逻辑资源维护。

### Toggle

属性：checked、text/onText/offText、colors、knob metrics、change。

额外编辑：状态预览；属性表即可。

抽象：`ChoiceControlEditor`。

### Slider

属性：value、min/max、orientation、step、track/fill/knob/focus/disabled colors、change。

额外编辑：画布内拖动滑块可修改 value；范围编辑器。

抽象：`RangeControlEditor`。

### Progress

属性：value、min/max、text、font、textColor、track/fill colors、showText。

额外编辑：值预览滑条；状态无交互。

抽象：`RangeDisplayEditor`。

### ScrollBar

属性：orientation、range/page/value、mode、colors、change。

额外编辑：拖动 thumb 预览；通常作为底层控件，不建议默认出现在控件库，可放高级分类。

抽象：`RangeControlEditor`。

### ScrollView / ScrollFrame / ScrollModel

属性：contentWidth/Height、offsetX/Y、wheelAxis、dragMode/contentDrag、scrollbarDrag、nestedScroll、scrollbarMode、colors、通用 viewport 属性。

额外编辑：content 画布、滚动偏移预览、content bounds 手柄、子控件拖放到 content slot。

抽象：`ViewportEditor`，提供 viewport slot 与 content coordinate mapping。

### ListView

属性：items、selected、multiSelect、itemHeight、scrollbarMode、font、colors、enabled items、adapter/binding、select/click。

额外编辑：Item 管理器，支持增删改排、禁用、选中默认项、批量导入；可切换“设计数据”和“运行时 adapter”模式；item owner draw 预览。

抽象：`CollectionEditor`，集合项不是普通 children，而是 `items[]` 数据模型。

### TreeView

属性：nodes、selectedId、expanded、checked、enabled、icon/check decoration、itemHeight、indent、scrollbarMode、font、colors、adapter/binding。

额外编辑：树节点管理器，支持新增子节点、移动层级、展开/折叠默认状态、checked 状态、图标和启用状态；画布中可点选节点。

抽象：`TreeCollectionEditor`，维护稳定 node id 和 parent id。

### TableView

属性：columns、rows/cells、selectionMode、selected row/cell、rowHeight、headerHeight、column widths、sort、cell adapter、colors、scrollbarMode。

额外编辑：列管理器、行/单元格数据编辑器、列宽拖拽、排序预览、选择模式预览。

抽象：`GridDataEditor`，将 columns、rows、cells 作为独立结构模型。

### TableGrid

属性：columns、rows、cell type、cell value、editor config、editMode、selection、merged/dirty/invalid/readonly 状态、colors。

额外编辑：表格设计器，支持直接编辑单元格、配置 editor 类型、枚举选项、颜色/file/image/picker metadata。

抽象：`EditableGridEditor`，后续可与 PropertyGrid 共用 cell editor schema。

### PropertyGrid

属性：categories、properties、type、value、defaultValue、flags、options、description、nameWidth、rowHeight、categoryHeight、descriptionMode、editMode、style/colors。

额外编辑：属性模型管理器，支持分类、属性、默认值、dirty/invalid/readonly、enum/options、action button、custom renderer；可复用 UIDesign 自己的属性表。

抽象：`PropertyModelEditor`，是 `EditableGridEditor` 的二列特化。

### ComboBox

属性：items/itemData、selectedIndex/value、enabledItems、font、popupHeight/maxHeight、popupPlacement、itemHeight、colors、itemColors、select。

额外编辑：选项管理器、默认选中、禁用项管理、弹层打开预览。

抽象：`OptionsControlEditor` + `OverlayPreviewEditor`。

### Menu

属性：items、separator、check/radio/submenu、shortcut、enabled/checked/default/danger、font、metrics、colors、select。

额外编辑：菜单结构编辑器，支持多级 submenu、快捷键、分隔线、radio group、打开预览。

抽象：`MenuModelEditor`，树形 items 数据模型。

### MenuBar

属性：top-level items、menus/submenus、font、metrics、colors、enabled、selected/open 状态。

额外编辑：菜单栏结构编辑器，顶层菜单和下拉菜单联动预览。

抽象：`MenuModelEditor` 的 menubar 入口。

### Toolbar

属性：items、button/toggle/separator、text、icon、tooltip、enabled、checked、overflow、metrics、colors、select。

额外编辑：工具项管理器，支持拖动排序、图标资源选择、overflow 预览。

抽象：`BarItemsEditor`。

### StatusBar

属性：sections、items、text/progress/spacer、width、align、colors、metrics。

额外编辑：状态栏 item 管理器，左/中/右分区，进度项预览。

抽象：`BarItemsEditor`。

### Tabs

属性：items、selected、enabledItems、dirtyItems、icons、closeButtons、scrollable、placement、scrollX、colors、close callback。

额外编辑：Tab 页面管理器，支持新增/删除/重排、dirty/disabled/closable、图标、tab content slot 映射。

抽象：`PagedCollectionEditor`。如果 tabs 承载真实 children，需要定义 `pages[] -> child slot`。

### Pager

属性：current、pageCount、total/pageSize、windowSize、first/last/prev/next text、font、metrics、colors、change。

额外编辑：属性表为主；页码窗口预览。

抽象：`RangeNavigationEditor`。

### Accordion

属性：sections、title、expanded、enabled、mode(single/multiple)、font、headerHeight、spacing、contentPadding、colors、select。

额外编辑：Section 管理器，每个 section 有 header 和 client slot；支持折叠预览、单开/多开策略。

抽象：`SectionedContainerEditor`，section client 是命名 slot。

### SplitLayout

属性：orientation、panes、pane size/min/max/collapsible、divider size、divider positions、colors。

额外编辑：画布直接拖 divider 调整比例；pane 管理；折叠状态预览。

抽象：`ResizablePaneEditor`。

### Window

属性：title、icon、closable/maximizable/collapsible/resizable、modal、dragAnywhere、client rect、min/max size、frame colors、chrome style、open/state。

额外编辑：窗口 chrome 预览、client slot 编辑、标题栏按钮状态、拖动/缩放手柄、modal backdrop 预览。

抽象：`ChromeContainerEditor`，client 是命名 slot。

### Popup

属性：owner、open、modal、autoClose、placement、anchorRect、anchorPoint、direction、gap、offset、matchOwnerWidth、consumeInside、close/focus policy、contentSize、scroll、background/border。

额外编辑：锚点编辑器、打开/关闭预览、placement 可视化、content slot。

抽象：`OverlayContainerEditor`。

### ColorPicker

属性：color/RGBA/hex、alphaEnabled、palette、font、colors、popupOpen、change。

额外编辑：调色板管理器、颜色选择弹层预览、hex/rgba 联动。

抽象：`ValueControlEditor` + `PaletteEditor` + `OverlayPreviewEditor`。

### DatePicker

属性：mode、nullable、value/rangeValue、limits、format、rangeSeparator、showSecond、firstDayOfWeek、defaultRangeSpan、colors、popupOpen、changing/change/commit/cancel/clear。

额外编辑：日期/时间弹层预览、范围选择预览、格式校验、min/max 限制编辑。

抽象：`ValueControlEditor` + `OverlayPreviewEditor`。

### Panel

属性：title、font、titleColor、titleAlign、background、clip、通用容器属性。

额外编辑：普通容器 children；标题快速编辑。

抽象：`ContainerEditor` + title 属性。

### DockPanel / DockLayout / DockWindow / DockPane

属性：regions、region portion/visible、dockWindows、window title/icon/closable/dockable/state、initial region/side/proportion、floating rect、autoHide、active pane tab、save/load state。

额外编辑：必须提供专用 Dock 设计器。功能包括：

- DockWindows 管理：新增、删除、重命名、设置 client slot、设置初始状态。
- Region 管理：document/left/right/top/bottom 比例、可见性、默认区域。
- 交互式停靠：画布中模拟拖动窗口，显示停靠指示器，提交到 `dockWindows`/state。
- Pane tab 管理：tab 排序、active tab、关闭/隐藏/autoHide。
- Floating 管理：浮动窗口 rect、z order。
- 状态快照：读取/写入 XValue 状态，作为设计时扩展字段或生成 XSON 初始布局。

抽象：`WorkbenchLayoutEditor`，这是最高复杂度控件，不能只放属性表。

### TimelineView

属性：layers、frames、spans、current frame、selection、frame count、frame width、layer width/header height、scroll、colors、renderers、context menu actions。

额外编辑：时间轴数据编辑器，支持图层增删改排、帧/key/span 编辑、拖动选择、右键菜单、锁定/隐藏状态。

抽象：`TimelineModelEditor`，类似专用小应用。

### MsgTip

属性：type、text、duration、icon、min/max width、offsetY、colors、open 状态。

额外编辑：消息样式预览；通常作为页面局部控件或服务节点。

抽象：`NotificationPreviewEditor`。

### MsgBox

属性：title、message、type、icon、buttons/customButtons、modal、open、result、colors。

额外编辑：按钮集合编辑器、modal/open 预览、结果码配置。

抽象：`DialogEditor`。

### InputBox

属性：title、prompt、initial/result、modal、open、colors、font。

额外编辑：对话框预览、默认输入值。

抽象：`DialogEditor`。

### Toast

属性：placement、direction、width、margin、gap、maxVisible、font、type colors、active/pending preview items。

额外编辑：服务配置面板和样例 toast 预览；不建议作为普通 widget 节点，作为 app/service 配置。

抽象：`ServiceEditor`。

### Tooltip

属性：text、delay、placement、style、dynamic resolver 名称。

额外编辑：作为所有 widget 的交互属性；提供 tooltip 预览。

抽象：通用属性组，不作为独立控件。

## 统一编辑面板

### 属性表

属性表由 schema 生成，每个 property 需要：

- key：写入 XSON 的字段名。
- displayName/category。
- value type：string/int/float/bool/color/enum/asset/rect/size/edges/binding/action。
- default value。
- editor type：text/number/color/options/resource/collection/action。
- visibility rule：例如仅当 `password=false` 时显示某些输入属性。
- validation rule。
- apply strategy：修改 AST 后重建预览、局部 patch、或只标记 dirty。

### 自定义编辑器启动入口

自定义编辑器不应绑定到单一入口。推荐统一为 `DesignAction`，由属性表、画布右键菜单、结构树右键菜单、工具栏按钮和双击行为共同调用。

常用入口：

- 属性表 picker：用于编辑某个属性值或资源引用，例如 image `srcRect`、Button icon、ComboBox items、PropertyGrid options、DockLayout state。
- 画布右键菜单：用于控件级操作，例如 “Edit Items...”、“Edit Nodes...”、“Edit Columns...”、“Edit Dock Layout...”。
- 结构树右键菜单：用于结构操作，例如新增 child、新增 section、移动 tab、添加 menu item、添加 tree child node。
- 画布双击：用于最常见的主编辑动作，例如 Label/Button 改文本、ListView 打开 item 管理器、DockWindow 进入 client slot。
- 顶部工具栏或面板按钮：用于全局设计器功能，例如打开 Style 管理器、Resource 管理器、Dock 工作台编辑器。

所有入口最终都调用同一套 action：

```c
typedef struct ui_design_action_t {
    const char* id;              /* edit.items / edit.nodes / edit.dockLayout */
    const char* title;
    int scope;                   /* property/node/slot/item/document */
    const char* propertyKey;     /* 属性表 picker 入口可填写 */
    uint32_t placement;          /* inspector/contextMenu/doubleClick/toolbar */
    int editorKind;              /* modal/popup/dockPanel/inline */
} ui_design_action_t;
```

执行时传入完整上下文：

```c
typedef struct ui_design_action_context_t {
    ui_document_t* doc;
    ui_node_t* node;
    const char* propertyKey;
    int itemIndex;
    int row;
    int column;
    xge_rect_t canvasRect;
    xvalue payload;
} ui_design_action_context_t;
```

这样同一个 “编辑 ListView items” 可以从属性表按钮、右键菜单或双击 ListView 启动，最终打开同一个 item 管理窗口，并写回同一份 `UIDocument`。

编辑器窗口形态按复杂度选择：

- Inline editor：简单文本、数字、颜色、枚举。
- Popup editor：小型资源选择、颜色选择、日期选择。
- Modal dialog：items/options/buttons/columns 等中等复杂模型。
- Docked tool window：DockLayout、TimelineView、TableGrid、资源库这类大型编辑器。

属性表 picker 适合“编辑某个属性”；右键菜单适合“编辑控件结构或执行控件动作”。两者都应该存在，但都只是 `DesignAction` 的不同入口，不应各自实现一套编辑逻辑。

### 结构面板

结构面板同时展示普通 widget tree 和控件内部结构：

- 普通 children：真实 XSON children。
- slot：Window client、Popup content、Accordion section client、DockWindow client。
- data items：List items、Tree nodes、Menu items、Toolbar items，不等同于 widget child。
- service nodes：Toast/Tooltip 等全局配置。

### 画布层

画布只处理通用选择、命中、拖放、resize、辅助线和 adorners。具体控件手感由控件编辑器提供：

- resize handles。
- padding/margin guides。
- grid/row/column/drop target。
- collection item hit proxy。
- overlay anchor handles。
- split divider handles。
- dock indicators。

### 专用编辑器

以下控件必须有专用编辑器：

- ListView：Item 管理。
- TreeView：Node 管理。
- TableView/TableGrid：列、行、单元格管理。
- PropertyGrid：分类、属性、editor config 管理。
- Menu/MenuBar：菜单树管理。
- Toolbar/StatusBar：bar item 管理。
- Tabs/Accordion：page/section 管理。
- SplitLayout：pane/divider 管理。
- DockLayout：工作台布局设计器。
- TimelineView：时间轴模型编辑器。
- Dialog/Overlay：打开态和锚点预览。

## 新控件接入流程

1. 在 `ControlEditorRegistry` 注册 type、role、category 和 capabilities。
2. 定义基础属性 schema。
3. 定义 child/slot/item 规则。
4. 如果是集合控件，实现 `buildStructure` 和 item CRUD action。
5. 如果需要画布专用手感，实现 adorners 和 canvas action。
6. 如果需要弹层或复杂模型，实现 special editor。
7. 添加 XSON round-trip 测试：默认节点、属性修改、结构修改、保存、重新加载。
8. 添加设计器 smoke：创建、选择、属性修改、撤销、预览。

## 建议的第一阶段范围

第一阶段不一次铺开所有复杂控件，但要完整实现 DockLayout 专用编辑器，用它验证复杂控件扩展机制。建议按风险递增：

1. 基础框架：UIDocument、Registry、Property schema、PreviewRuntime、Undo stack。
2. 简单控件：label/button/input/image/separator/checkbox/radio/toggle/slider/progress。
3. 容器布局：panel/row/column/stack/grid/dock/absolute/scrollView。
4. MVP 集合控件：comboBox/listView/treeView/tabs。
5. MVP 数据控件：propertyGrid。
6. MVP 工作台控件：dockLayout，并实现完整专用编辑器，用来验证扩展机制。
7. 后续再接入 tableView/tableGrid、menu/menuBar、toolbar/statusBar、window/popup/dialog、colorPicker/datePicker/numericInput/textEdit、timelineView。

这个顺序能保证属性表和画布基础先稳定，同时用 DockLayout 作为复杂控件样板验证专用编辑器机制，避免后续复杂控件各自发明入口和写回链路。
