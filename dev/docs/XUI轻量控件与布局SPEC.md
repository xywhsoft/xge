# XUI 轻量控件与布局 SPEC

本文档定义 XUI 下一阶段的控件库与布局系统方向。目标不是实现一个大而全的桌面 UI 框架，而是实现一个小而精美、运行迅速、机制克制、便于组合的 retained DUI 库。

> 2026-05-07 口径更新：本文保留为历史阶段记录。后续新增实现、重构和验收以 `XUI Widget V2基础设计.md` 与 `XUI Widget V2基础SPEC.md` 为准；本文中的旧控件分类与 z/order 成熟度判断不再作为当前路线依据。

## 1. 设计目标

- XUI 采用 retained widget tree 和类 HTML 的组合式布局思想。
- 核心控件保持原子化，复合界面通过布局组合实现。
- 运行速度优先，常见路径不能被少数复杂控件拖慢。
- 代码规模保持克制，每个控件一个独立源码文件。
- 默认视觉要足够精美，但主题系统保持轻量。
- 不实现完整 CSS，不实现浏览器级布局引擎。
- 核心控件以真实通用场景和 Widget V2 成熟度为准；Toolbar、StatusBar、TreeView、TableView、PropertyGrid 等已进入 XGE 内置 XUI 的历史实现，后续按 Widget V2 重验；ColorPicker 与 DatePicker 已按基础控件口径补齐并重验。
- 大数据控件优先虚拟化，不通过创建大量 widget 暴力实现。

## 2. 非目标

以下能力暂不作为 XUI 核心目标：

- 完整 CSS selector、cascade、media query。
- 完整 Flexbox / CSS Grid 兼容。
- 复杂约束布局。
- Ribbon、Docking IDE 框架、完整 Property IDE 框架。
- 内建复杂富文本排版引擎。
- 内建完整输入法，只依赖系统 IME 事件。
- 每种业务界面都做成独立复合控件。

## 3. 布局系统方向

当前布局已具备 absolute、row、column、stack、grid-lite、anchor、margin、padding、align、DIP、percent、content、grow、min/max size；历史实现中也有局部 z/order 和 clip，但它们不再视为成熟能力，必须按 Widget V2 重新设计和验收。

下一阶段优先重做 Widget 基础层、盒模型、clip、Z 序、事件、焦点和 ScrollViewBase；Flex-lite 只在这些基础稳定后继续补强。

### 3.1 必须补强

- 两阶段布局：`measure` 和 `arrange`。
- Widget 缓存 `desiredSize`，避免重复测量。
- Row/Column 支持 `gap`。
- Row/Column 支持主轴对齐：start、center、end、space-between。
- Row/Column 支持交叉轴对齐：start、center、end、stretch。
- `content` 尺寸可以由子元素汇总得到。
- Dirty layout 只重新计算必要子树。
- Scroll container 的 content rect、scroll offset、hit test 坐标语义统一。
- Overlay root，用于 Popup、Tooltip、ComboBox 下拉层、ContextMenu、Window。
- Widget role：Control、Container、Viewport、Overlay。
- `overflow`：visible、hidden、clip、scroll。
- 统一 `layer > zIndex > treeOrder` 的绘制、hit test 和事件目标选择规则。
- ScrollViewBase / VirtualScrollViewBase 作为滚动类控件基础设施。

### 3.2 可选补强

- Row/Column wrap。
- Grid-lite 支持 column gap、row gap、固定行高和 grow 行高。
- Layout debug overlay，用于显示 widget rect/content rect/margin/padding。

### 3.3 暂不实现

- 完整 flex-basis / flex-shrink / order。
- 完整 CSS Grid tracks、span、auto placement。
- 复杂响应式断点系统。
- 自动动画布局。

## 4. 控件拆分规则

每个控件使用一个单独源码文件实现。

建议源码结构：

```text
src/xge_xui_internal.h
src/xge_xui_button.c
src/xge_xui_label.c
src/xge_xui_image.c
src/xge_xui_input.c
src/xge_xui_text_edit.c
src/xge_xui_checkbox.c
src/xge_xui_radio.c
src/xge_xui_switch.c
src/xge_xui_slider.c
src/xge_xui_progress.c
src/xge_xui_scrollbar.c
src/xge_xui_scroll_view.c
src/xge_xui_list_view.c
src/xge_xui_popup.c
src/xge_xui_menu.c
src/xge_xui_combo_box.c
src/xge_xui_separator.c
src/xge_xui_split_layout.c
src/xge_xui_tabs.c
```

`xge_xui_internal.h` 只暴露 XUI 内部 helper，不进入公共 API。公共声明仍保留在 `xge.h`。

## 5. 控件分类

### 5.1 核心原子控件

这些控件应进入 XUI 核心：

- `Label`
- `Button`
- `Button icon mode`
- `Image`
- `Input`
- `TextEdit`
- `CheckBox`
- `RadioButton`
- `Toggle`
- `Slider`
- `Progress`
- `ScrollBar`
- `ScrollView`
- `ListView`
- `ComboBox`
- `Popup`
- `Menu`
- `Tooltip`
- `Window`
- `Separator`
- `SplitLayout divider`
- `Tabs`

### 5.2 复合控件当前口径

Toolbar、StatusBar 等应用骨架控件已有第一版实现，旧的固定排除判断已失效。MenuBar、Ribbon、Docking IDE 框架仍作为可选或后续能力，不进入 Widget 基础层。复合控件是否进入核心，取决于 Widget V2 基础层成熟后是否能稳定覆盖通用场景。

- Toolbar、StatusBar：已有历史实现，按 Widget V2 重验后再决定成熟度。
- MenuBar、Ribbon、Docking IDE 框架：作为可选高级应用框架能力。
- Sidebar、Header、Footer、Inspector、SettingsPage、PropertyPanel：优先用 Container + 布局组合表达。

### 5.3 控件成熟度口径

- TreeView、TableView、PropertyGrid 已有第一版实现，但需按 Widget V2 重验并补齐成熟行为；ColorPicker 与 DatePicker 已补齐完整基础控件能力并重验。
- DatePicker 是核心基础应用控件，当前已支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 lab 验证。
- RichTextView、CodeEditor、NodeGraph、Timeline 作为可选高级组件，暂不进入核心基础层。

## 6. 编辑框设计

编辑框拆成两个控件：

- `Input`：单行编辑框。
- `TextEdit`：多行编辑框，目标能力对标记事本。

两者共享底层文本编辑内核，但控件实现分开。这样可以避免单行控件被多行能力拖复杂，也可以让多行控件独立演进。

### 6.1 共享文本内核

建议保留并扩展当前 `xge_xui_text_t`，形成轻量文本编辑内核：

- UTF-8 文本存储。
- 光标位置。
- 选择范围。
- 插入文本。
- 删除前一个字符。
- 删除后一个字符。
- IME composition 文本。
- 文本变更版本号。

后续可扩展：

- 行索引缓存。
- undo/redo 栈。
- 剪贴板接入。
- 单词边界查询。

### 6.2 Input 单行编辑框

`Input` 只承担单行场景：

- 单行文本输入。
- 系统 IME 上屏。
- composition 显示。
- 光标显示。
- 鼠标点击定位光标。
- 拖拽选择。
- Backspace/Delete。
- Left/Right。
- Home/End。
- Ctrl+Left/Ctrl+Right。
- Ctrl+A。
- Ctrl+C/Ctrl+X/Ctrl+V。
- 默认上下文菜单：全选、剪切、复制、粘贴、删除。
- placeholder。
- password 模式。
- readonly 模式。
- disabled 模式。
- 横向滚动，使光标始终可见。

不在 `Input` 内实现：

- 自动换行。
- 多行选择绘制。
- 行号。
- 垂直滚动。

### 6.3 TextEdit 多行编辑框

`TextEdit` 是类似记事本的多行编辑框，独立实现：

- 多行文本输入。
- 换行输入。
- 自动换行可选。
- 水平滚动。
- 垂直滚动。
- 光标上下左右移动。
- Home/End。
- Ctrl+Home/Ctrl+End。
- PageUp/PageDown。
- 鼠标点击定位。
- 鼠标拖拽多行选择。
- Shift+方向扩展选择。
- Ctrl+A/C/X/V。
- Backspace/Delete。
- 系统 IME composition。
- 光标自动滚入可视区域。
- 文本选择高亮。
- 行缓存，避免每帧重新扫描全文。
- undo/redo，至少支持文本插入、删除、粘贴。
- 默认上下文菜单：全选、剪切、复制、粘贴、删除。

暂不要求：

- 富文本。
- 语法高亮。
- 多光标。
- 行号。
- 代码折叠。
- 大文件 mmap 编辑。

### 6.4 Input 与 TextEdit 的关系

`Input` 和 `TextEdit` 不合并成一个控件，但共享内部文本函数。原因：

- 单行输入是高频轻量控件，必须保持简单和快。
- 多行编辑涉及行布局、滚动、选择、undo/redo，复杂度明显更高。
- 分开实现能减少 API 分支和状态组合。

## 7. 控件状态模型

所有交互控件应尽量复用统一状态：

- normal
- hover
- active
- focused
- disabled
- checked
- selected
- readonly

视觉上第一阶段只要求三状态可靠：

- normal
- hover
- active

focused 可用于键盘导航和输入控件，但普通按钮不强制使用独立焦点底色。

## 8. Popup / Overlay

Popup 是 ComboBox、ContextMenu、Window 等公开浮层控件的基础设施。Tooltip 是 widget 内部机制，只复用 overlay root、layer/zIndex/treeOrder 和 screen clamp，不作为 Popup 控件实例实现。

要求：

- Popup 绘制在 overlay root。
- Popup 不受普通父 widget clip 限制。
- Popup 可绑定 owner widget。
- Popup 支持自动关闭。
- Popup 支持点击外部关闭。
- Popup 支持 ESC 关闭。
- Popup 默认必须走统一 OverlayManager；默认叠放使用 layer/zIndex/treeOrder，手动 z 只作为特殊覆盖策略。

`Menu` 是基于 Popup 和 ListView 的轻量菜单控件，用于 ContextMenu 等场景。要求：

- 支持按坐标弹出并约束在窗口内。
- 支持点击外部和 ESC 关闭。
- 支持 enabled/disabled item。
- disabled item 置灰且不可触发。
- 可被 Input/TextEdit 用作默认编辑上下文菜单。

暂不要求：

- 复杂动画。
- 多窗口 popup。
- 跨 monitor 定位。

## 9. ScrollBar / ScrollView

`ScrollBar` 应作为独立控件存在，`ScrollView` 和 `ListView` 可复用它的行为逻辑或绘制逻辑。

2026-05-07 后，旧 `ScrollView` 口径被 ScrollViewBase / VirtualScrollViewBase 取代。ScrollViewBase 负责普通滚动视图基础设施，VirtualScrollViewBase 在其上提供可见范围、slot 复用和数据绑定基础。滚轮方向必须可配置为 vertical/y、horizontal/x、both；鼠标拖拽滚动默认关闭，由 `contentDrag` 或 `dragMode` 显式打开，避免干扰地图编辑器、画布编辑器等需要完整鼠标事件的控件；滚动条 thumb 拖拽仍默认可用。

`ScrollBar` 要求：

- 横向/纵向。
- min/max/page/value。
- 鼠标拖动 thumb。
- 点击 track 翻页。
- 滚轮可选。
- disabled。

`ScrollView` 要求：

- 统一滚动偏移。
- 子元素 hit test 坐标正确。
- 子元素绘制受 content clip 限制。
- 可选择显示或隐藏横纵滚动条。

## 10. ListView

`ListView` 继续保持轻量虚拟化，不为每一行创建 widget。

要求：

- 可见行虚拟绘制。
- item count。
- item text provider 或 item array。
- selected。
- hover row。
- mouse wheel。
- keyboard up/down/page。
- onSelect 回调。

暂不要求：

- 多列。
- 树形层级。
- 每行复杂 widget。

## 11. 主题系统

主题系统保持轻量，不做 CSS。

建议提供：

- 全局 theme token。
- 控件级颜色 override。
- 控件状态颜色。
- radius、padding、spacing、border width。
- font。

暂不提供：

- selector。
- inheritance cascade。
- runtime stylesheet parser。

## 12. 性能约束

XUI 布局与控件实现必须遵守：

- 默认帧不应全树 layout。
- paint-only 状态变化不能触发布局。
- 文本测量必须可缓存。
- `desiredSize` 必须可缓存。
- 大列表必须虚拟化。
- 不为几千行列表创建几千个 widget。
- 控件 paint 不应强制 flush。
- UI primitive 应尽量走 XGE 现有 shape/sprite batching。

参考目标：

- 几十个控件：布局和绘制成本应接近无感。
- 几百个控件：只要没有全量动态布局，应稳定流畅。
- 几千项数据：必须通过虚拟化控件承载。

## 13. 实施阶段

### 阶段 A：工程整理

- 新增 `xge_xui_internal.h`。
- 将现有控件拆成每控件一个源码文件。
- 保持公共 API 不大幅变化。
- 确保现有测试和人工验证通过。

### 阶段 B：布局够用升级

- 实现 measure/arrange。
- 增加 desired size cache。
- Row/Column 增加 gap。
- Row/Column 增加 justify。
- content size 可由 children 汇总。
- Dirty layout 跳过干净子树。
- 增加 layout debug 示例。

### 阶段 C：交互基础设施

- 完善 hover/capture/focus。
- 增加 keyboard focus traversal。
- 增加 Enter/Space 激活。
- 增加 Popup/Overlay。
- 增加 ScrollBar。

### 阶段 D：核心控件补全

- CheckBox。
- RadioButton。
- Toggle。
- Button icon mode。
- Separator。
- SplitLayout divider。
- Tabs。
- ComboBox。
- Tooltip。

### 阶段 E：编辑框增强

- 完善 Input。
- 新增 TextEdit。
- 完成剪贴板。
- 完成 undo/redo。
- 完成多行选择和滚动。

## 14. 验收标准

每个控件至少需要：

- 一个自动测试或逻辑测试。
- 一个可运行示例。
- 一组人工验证步骤。
- normal/hover/active/disabled 基础状态验证。
- 与 layout、clip、focus、capture 的组合验证。

布局系统升级至少需要：

- nested row/column。
- content size。
- grow。
- gap。
- justify。
- scroll container。
- overlay popup。
- resize。
- dirty layout 行为验证。

## 15. 当前决策

- XUI 保持小而精，不做大而全。
- 复合界面优先用布局组合，不新增固定复合控件。
- 单行编辑框和多行编辑框拆分为两个控件。
- 多行编辑框以记事本能力为目标，但不进入代码编辑器和富文本范围。
- 布局只升级到 Flex-lite，不实现完整 CSS/Flex/Grid。

## 16. 任务跟踪规则

本节用于跟踪 XUI 后续开发进度。每完成一项实现、测试或人工验证，都应同步更新本节对应勾选项。

勾选规则：

- `[ ]` 未开始或尚未完成。
- `[x]` 已完成，并且至少通过构建验证。
- 若任务包含人工验证，必须在人工验证通过后才能勾选。
- 若任务拆分出新子任务，应追加到对应阶段，不要只在聊天中记录。
- 若任务范围发生变化，应先更新本 SPEC，再继续实现。

完成口径：

- 工程整理类任务：代码结构调整完成，构建通过，现有示例不回退。
- 布局类任务：自动测试或示例覆盖主要行为，人工观察无明显错位。
- 控件类任务：至少有初始化、状态变化、绘制、事件响应和销毁路径。
- 输入编辑类任务：必须包含键盘、鼠标、IME 和剪贴板相关验证。

## 17. 总体进度

- [x] 确认 XUI 方向为小而精、快速、够用、可组合。
- [x] 确认不做完整 CSS/Flex/Grid。
- [x] 历史决策记录：布局型复合界面的旧分类已被 2026-05-07 Widget V2 新分类取代；Toolbar、StatusBar、TreeView、TableView、PropertyGrid、ColorPicker 等按 Widget V2 口径重新分类。
- [x] 确认单行编辑框和多行编辑框拆成两个控件。
- [x] 完成 XUI 源码工程整理。说明：阶段 A 细项已完成，XUI 内部 helper 边界、源码拆分、构建脚本和示例验证均已落地。
- [x] 完成 Flex-lite 布局升级。说明：阶段 B 细项已完成，measure/arrange、dirty layout、Row/Column gap/justify/align、content size、grow 和布局验证均已落地。
- [x] 完成 Popup/Overlay 基础设施。
- [x] 完成核心原子控件补全。
- [x] 完成 Input 单行编辑框增强。
- [x] 完成 TextEdit 多行编辑框。
- [x] 完成 XUI 综合人工验证示例。

## 18. 阶段 A：工程整理任务

- [x] 新增 `src/xge_xui_internal.h`。
- [x] 建立 XUI 内部 helper 整理边界。
- [x] 将 XUI 内部 helper 从公共实现文件中逐步整理到 internal header。
- [x] 拆分 `Button` 到 `src/xge_xui_button.c`。
- [x] 拆分 `Label` 到 `src/xge_xui_label.c`。
- [x] 拆分 `Image` 到 `src/xge_xui_image.c`。
- [x] 拆分 `Input` 到 `src/xge_xui_input.c`。
- [x] 拆分 `Toggle` 到独立文件，后续逐步收敛为 `CheckBox/Toggle`。
- [x] 拆分 `Slider` 到 `src/xge_xui_slider.c`。
- [x] 拆分 `Progress` 到 `src/xge_xui_progress.c`。
- [x] 拆分 `Panel` 到 `src/xge_xui_panel.c`。
- [x] 拆分 `ScrollView` 到 `src/xge_xui_scroll_view.c`。
- [x] 拆分 `ListView` 到 `src/xge_xui_list_view.c`。
- [x] 调整构建脚本，纳入所有新增 XUI 源码文件。
- [x] 确认单头文件生成流程包含新增 XUI 源码文件。
- [x] 运行 `build_test.bat` 并通过。
- [x] 运行 `examples\xui_bridge\build.bat` 并通过。
- [x] 人工验证现有 XUI 示例无回退。

## 19. 阶段 B：布局升级任务

- [x] 为 widget 增加 `desiredSize` 缓存。
- [x] 增加 layout dirty 版本或标记，避免重复测量。
- [x] 实现 `measure` 阶段。
- [x] 实现 `arrange` 阶段。
- [x] 保持 absolute 布局兼容现有行为。
- [x] Row 布局支持 `gap`。
- [x] Column 布局支持 `gap`。
- [x] Row 布局支持主轴 justify：start、center、end、space-between。
- [x] Column 布局支持主轴 justify：start、center、end、space-between。
- [x] Row/Column 支持交叉轴 align：start、center、end、stretch。
- [x] `content` width 可由 children 汇总。
- [x] `content` height 可由 children 汇总。
- [x] Dirty layout 跳过干净子树。
- [x] 布局更新不破坏 `tLocalRect`。
- [x] 新增布局验证示例。
- [x] 新增 nested row/column 验证。
- [x] 新增 content size 验证。
- [x] 新增 grow/gap/justify 验证。
- [x] 新增 resize 验证。
- [x] 人工验证布局示例无错位、漂移、闪烁。

## 20. 阶段 C：交互基础设施任务

- [x] 修复 button hover 离开后状态不恢复的问题。
- [x] 将 button 视觉先收敛为 normal/hover/active 三状态。
- [x] 增加通用 pointer enter/leave 语义。
- [x] 增加 Tab / Shift+Tab 焦点遍历。
- [x] 增加 Enter 激活当前焦点控件。
- [x] 增加 Space 激活当前焦点控件。
- [x] 增加 ESC 取消当前 capture/popup。
- [x] 增加 capture lost/cancel 处理。
- [x] 增加 Popup/Overlay root。
- [x] Popup 支持 owner widget。
- [x] Popup 支持点击外部关闭。
- [x] Popup 支持 ESC 关闭。
- [x] Popup 不受普通父 widget clip 限制。
- [x] 新增 `Menu` 控件，基于 Popup/ListView。
- [x] `Menu` 支持 disabled item 置灰和不可触发。
- [x] 新增 Popup 人工验证示例。

## 21. 阶段 D：核心控件任务

- [x] `Button icon mode`：实现图标按钮。
- [x] `CheckBox`：实现复选框。
- [x] `RadioButton`：实现单选按钮。
- [x] `RadioGroup`：实现同组互斥逻辑或辅助 API。
- [x] `Toggle`：实现开关控件。
- [x] `Separator`：实现分隔线控件。
- [x] `SplitLayout divider`：实现拖拽分割条。
- [x] `Tabs`：实现标签页控件。
- [x] `ScrollBar`：实现独立滚动条控件。
- [x] `ScrollBar`：支持 horizontal/vertical。
- [x] `ScrollBar`：支持 thumb 拖拽。
- [x] `ScrollBar`：支持 track 翻页。
- [x] `ScrollView`：接入或复用 `ScrollBar` 行为。
- [x] `ListView`：增加 hover row。
- [x] `ListView`：增加键盘上下选择。
- [x] `ListView`：增加 PageUp/PageDown。
- [x] `ComboBox`：基于 Popup 实现下拉选择。
- [x] `Tooltip`：作为 widget 属性和 context 内部 overlay 服务实现提示。
- [x] `Window`：完善 modal、ESC、关闭按钮行为。
- [x] 每个新增控件提供一个最小示例。
- [x] 每个新增控件提供人工验证步骤。

## 22. 阶段 E：Input 单行编辑框任务

- [x] `Input` 支持 placeholder。
- [x] `Input` 支持 password 模式。
- [x] `Input` 支持 readonly 模式。
- [x] `Input` 支持 disabled 模式。
- [x] `Input` 支持 Home/End。
- [x] `Input` 支持 Ctrl+Left/Ctrl+Right。
- [x] `Input` 支持 Ctrl+A。
- [x] `Input` 支持 Ctrl+C。
- [x] `Input` 支持 Ctrl+X。
- [x] `Input` 支持 Ctrl+V。
- [x] `Input` 支持横向滚动，光标始终可见。
- [x] `Input` 支持双击选词。
- [x] `Input` 支持光标闪烁。
- [x] `Input` 保持系统 IME composition 正常。
- [x] `Input` 支持默认上下文菜单。
- [x] `Input` 默认上下文菜单支持 Select All/Cut/Copy/Paste/Delete。
- [x] `Input` 默认上下文菜单与选区、readonly、剪贴板、password 状态联动。
- [x] 新增 Input 综合示例。
- [x] 人工验证英文输入。
- [x] 人工验证中文 IME。
- [x] 人工验证剪贴板。
- [x] 人工验证选择、删除、光标移动。

## 23. 阶段 F：TextEdit 多行编辑框任务

- [x] 新增 `src/xge_xui_text_edit.c`。
- [x] 定义 `xge_xui_text_edit_t`。
- [x] `TextEdit` 支持多行文本显示。
- [x] `TextEdit` 支持换行输入。
- [x] `TextEdit` 支持水平滚动。
- [x] `TextEdit` 支持垂直滚动。
- [x] `TextEdit` 支持可选自动换行。
- [x] `TextEdit` 支持鼠标点击定位光标。
- [x] `TextEdit` 支持鼠标拖拽多行选择。
- [x] `TextEdit` 支持 Left/Right/Up/Down。
- [x] `TextEdit` 支持 Home/End。
- [x] `TextEdit` 支持 Ctrl+Home/Ctrl+End。
- [x] `TextEdit` 支持 PageUp/PageDown。
- [x] `TextEdit` 支持 Shift+方向扩展选择。
- [x] `TextEdit` 支持 Ctrl+A。
- [x] `TextEdit` 支持 Ctrl+C/Ctrl+X/Ctrl+V。
- [x] `TextEdit` 支持 Backspace/Delete。
- [x] `TextEdit` 支持系统 IME composition。
- [x] `TextEdit` 支持光标自动滚入可视区域。
- [x] `TextEdit` 支持多行选择高亮。
- [x] `TextEdit` 支持行缓存，避免每帧扫描全文。
- [x] `TextEdit` 支持 undo/redo。
- [x] `TextEdit` 支持双击选词。
- [x] `TextEdit` 支持默认上下文菜单。
- [x] `TextEdit` 默认上下文菜单支持 Select All/Cut/Copy/Paste/Delete。
- [x] `TextEdit` 默认上下文菜单与选区、readonly、剪贴板状态联动。
- [x] 新增 TextEdit 综合示例。
- [x] 人工验证基础输入能力对标记事本。
- [x] 人工验证中文 IME。
- [x] 人工验证剪贴板。
- [x] 人工验证大段文本滚动和选择。

## 24. 阶段 G：测试与人工验证任务

- [x] 新增 XUI 综合控件示例。
- [x] 新增 XUI 布局验证示例。
- [x] 新增 XUI Popup 验证示例。
- [x] 新增 XUI 编辑框验证示例。
- [x] 新增 XUI 验证程序聚合构建脚本。
- [x] 新增 XUI 验证程序并行聚合构建脚本。
- [x] 新增 XUI Windows 人工验证顺序运行脚本。
- [x] 新增 XUI Windows 人工验证记录模板。
- [x] 更新 `dev/docs/人工冒烟测试流程.md` 中的 XUI 验证项。
- [x] 自动测试覆盖 widget tree。
- [x] 自动测试覆盖 layout measure/arrange。
- [x] 自动测试覆盖 focus/capture/hover。
- [x] 自动测试覆盖 Input 文本内核。
- [x] 自动测试覆盖 ScrollBar 数值逻辑。
- [x] Windows 人工验证通过。

## 25. 后续任务分类

以下任务按 Widget V2 后的新成熟度口径重新分类：

- [x] TreeView：已按 Widget 口径重验。说明：通过 `VirtualScrollViewBase` 复用滚动、visible index、hover/focus/selected 镜像和 capture。
- [x] TableView：已按 Widget 口径重验。说明：通过 `VirtualScrollViewBase` 复用 row count、hover/focus/selected row、滚动和滚动条状态。
- [x] PropertyGrid：已按 Widget 口径重验。说明：通过 `VirtualScrollViewBase` 复用可见 row、hover/focus/selected visible index、滚动和滚动条状态。
- [x] ColorPicker：已补齐完整基础控件能力并按 Widget 口径重验。说明：支持 palette、RGBA 数字字段、hex 字段、键盘编辑、提交/取消、错误态和编辑态 IME 策略。
- [x] DatePicker：核心基础应用控件，已按 Widget 口径补齐并重验。说明：支持单 widget 月历、日期范围、月切换、鼠标选择、键盘导航、XSON `datePicker` 和 lab 验证。
- [x] RichTextView：可选高级组件路线，不阻塞核心基础层；依赖 TextEdit/ScrollViewBase/选择与格式绘制基础设施。
- [x] CodeEditor：可选高级组件路线，不阻塞核心基础层；依赖 TextEdit/VirtualScrollViewBase/行号、语法着色和查找高亮扩展。
- [x] NodeGraph：可选高级组件路线，不阻塞核心基础层；依赖 ScrollViewBase、拖拽、命中、选择和自绘节点/连线。
- [x] Timeline：可选高级组件路线，不阻塞核心基础层；依赖 VirtualScrollViewBase、缩放、选择和自绘刻度/轨道。
