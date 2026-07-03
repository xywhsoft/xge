# XUI Layout System Design

本文记录 XUI 核心布局系统设计。XUI 布局目标是 APP 优先、文档够用、控件级复杂布局外置，避免把基础布局引擎做成浏览器级重量。

## 1. Design Position

XUI 的核心布局只回答一个问题：

```text
一个 widget 如何安排自己的直接 children
```

SplitLayout、DockPanel、ScrollView、Viewport、VirtualList、TableView、DocumentView 等属于控件级或标准库级能力，不进入核心布局枚举。它们可以复用核心布局算法，但它们自己的滚动、虚拟化、拖拽、停靠和数据管理不应污染基础 widget layout。

XUI 的优先级：

1. APP 布局便利性和确定性。
2. 文档布局能力补齐。
3. 复杂控件基于核心布局扩展。
4. 不追求完整 HTML/CSS 布局规范。

## 2. Core Layout List

XUI 核心布局收敛为：

```text
absolute
stack
row
column
dock
grid
table
flow
```

这些 layout 覆盖基础 APP 和常见文档排版需求。

| Layout | Primary Scenario |
| --- | --- |
| `absolute` | 固定坐标、anchor、游戏 HUD、局部精确摆放 |
| `stack` | 层叠、背景层、遮罩、badge、多个 child 共用同一 slot |
| `row` | 横向排列、工具栏、状态栏、表单行 |
| `column` | 纵向排列、设置页、属性面板、表单分组 |
| `dock` | APP shell、top/bottom/left/right/fill 主框架 |
| `grid` | 轻量固定网格、图标墙、按钮矩阵、固定规格卡片 |
| `table` | 二维大小协商、表单、属性页、复杂设置面板 |
| `flow` | div/span 式文档流、自动换行、图文/控件混排 |

## 3. Non-Core Layouts

以下能力不进入核心 layout enum：

| Capability | Reason |
| --- | --- |
| Split layout | 拖拽、比例、最小尺寸、预览线属于控件行为 |
| DockPanel / IDE docking | 停靠窗口、pane、tab、floating、auto-hide 是复杂控件系统 |
| Viewport / ScrollView | scroll offset、scrollbar、clip、content size 属于 viewport 控件 |
| VirtualList / TableView | 数据、虚拟化、选择、编辑、可见行管理属于数据控件 |
| Popup placement | anchor、flip、fit、screen clamp 属于 overlay/placement 基础设施 |
| DocumentView | 文本选择、链接、锚点、滚动、分页等属于文档控件 |

这些控件可以使用 core layout，也可以调用 shared layout engine，但不能反向把自己的状态机塞进基础 widget layout。

## 4. Row and Column Stay Separate

XUI 不把 `row` 和 `column` 在公开 API 上合并成 `linear`。

原因：

- UI 设计心智中，横排和竖排是最直观的第一层判断。
- APP 快速排布时，`row` / `column` 比 `linear + direction` 更清楚。
- 现有 XUI 的横向/纵向布局心智成熟，应该保留。

内部实现可以共享 linear 算法，但公开语义保持：

```text
row     horizontal layout
column  vertical layout
```

## 5. Formatting Context Rule

XUI 布局系统采用 formatting context 边界：

```text
父 layout 决定当前 formatting context
child 只使用该 context 支持的 participation 属性
不适用于当前 context 的属性被忽略，debug 模式可给 warning
```

例如：

- `display: inline` 只在 `flow` 中生效。
- `lineBreak` / `breakBefore` 只在 `flow` 中生效。
- `dock: left` 只在父 layout 是 `dock` 时生效。
- `grid row/column/span` 只在父 layout 是 `grid` 或 `table` 的对应模式中生效。

这样可以避免 `row` 被文档流属性污染，也避免布局系统滑向完整 flexbox/browser layout。

## 6. Shared Layout Kernel

不同 layout 共享底层计算部件：

- measure child
- resolve px/dip/percent/content/grow/star
- apply min/max
- margin and padding
- align in slot
- baseline metrics
- content size reporting
- overflow bounds reporting

但公开语义保持分离：

```text
row/column: deterministic APP arrangement
flow: content-driven document arrangement
table: two-dimensional track negotiation
```

## 7. Measurement Protocol

所有 layout 统一走：

```text
measure -> arrange -> commit
```

Widget 或 control 在 measure 阶段返回：

```text
min size
preferred size
max size
baseline
content size
overflow bounds
```

说明：

- `min size` 是不破坏基本可用性的最小尺寸。
- `preferred size` 是自然尺寸。
- `max size` 为 0 或特殊值时表示不限制。
- `baseline` 用于文本、输入框、图标文字混排。
- `content size` 可大于 assigned size，供 viewport 或 overflow report 使用。
- `overflow bounds` 包含视觉外扩，例如 focus ring、shadow、glow。

## 8. Overflow Policy

基础 widget 不自动创建 scrollbar。

Widget 可表达 overflow 策略：

```text
visible
hidden
clip
report
```

- `visible`: 允许绘制溢出。
- `hidden`: 隐藏溢出区域。
- `clip`: 裁剪到 content rect。
- `report`: 不滚动、不创建 scrollbar，但报告 content size / overflow bounds 给父控件或 viewport 控件。

`flow` 普通 layout 可以报告内容高度超过 assigned height；是否滚动由 `FlowView`、`DocumentView` 或其它 viewport 控件决定。

## 9. Absolute Layout

`absolute` 使用 child 的 local rect、anchor、margin 布局。

适合：

- 游戏 HUD。
- 浮层内部固定定位。
- 局部精确摆放。
- 需要 anchor 的特殊区域。

不适合：

- 大多数 APP 页面。
- 文档流。
- 响应式内容区。

Child participation:

```text
x / y / width / height
anchor left/top/right/bottom
margin
align
```

## 10. Stack Layout

`stack` 把所有 child 放进同一个父 content slot，并按 Z order/tree order 叠放。

适合：

- 背景层 + 内容层。
- 遮罩。
- badge。
- loading overlay。
- 简单装饰层。

Child participation:

```text
width / height
margin
align
zIndex / layer
```

## 11. Row Layout

`row` 横向排列 children。

适合：

- 工具栏。
- 状态栏。
- 表单行。
- 按钮组。
- label + input + action。

Child participation:

```text
width / height
min / max
margin
grow
alignX / alignY
baselineAlign
```

Rules:

- 固定、percent、content 先占位。
- `grow` 分配剩余横向空间。
- `gap` 位于相邻 child 之间。
- `justify` 只在没有 grow 或 grow 未耗尽剩余空间时生效。
- `baseline` 可用于文本/输入框对齐。
- 不支持 child 自己请求换行。

若需要 row 内部某块内容自然换行，应把该块作为 `flow` child：

```text
row
  label fixed
  flow grow
    inline text
    inline link
    inline badge
```

## 12. Column Layout

`column` 纵向排列 children。

适合：

- 设置页。
- 纵向表单。
- 属性面板。
- 面板内容区。
- APP 页面主内容组织。

Child participation:

```text
width / height
min / max
margin
grow
alignX / alignY
```

Rules:

- 固定、percent、content 先占位。
- `grow` 分配剩余纵向空间。
- `gap` 位于相邻 child 之间。
- `justify` 只在没有 grow 或 grow 未耗尽剩余空间时生效。
- 不做文档流自动重排。

## 13. Dock Layout

`dock` 按 child 顺序消耗父 content rect 的剩余区域。

适合：

- APP shell。
- top header。
- bottom status bar。
- left/right sidebar。
- center/fill content。

Child participation:

```text
dock: top / bottom / left / right / fill
width / height
margin
align
```

Rules:

- child 顺序重要。
- top/bottom 消耗高度。
- left/right 消耗宽度。
- fill 使用当前剩余区域。
- dock 只做 shell 分区，不做 IDE DockPanel。

## 14. Grid Layout

`grid` 是轻量 APP 网格，不是 CSS Grid。

适合：

- 图标墙。
- 按钮矩阵。
- 固定规格卡片。
- 快捷功能区。
- 简单展示面板。

Rules:

- 固定列数。
- 固定行高或方形行高。
- 顺序填充。
- 支持简单 `columnSpan`。
- 高性能、低心智负担。

明确不做：

- track sizing。
- auto placement 完整规则。
- dense packing。
- grid-template-area。
- implicit infinite grid。

需要二维大小协商时使用 `table`。

## 15. Table Layout

`table` 是二维大小协商布局，定位类似 WPF Grid 的核心子集，不是数据表格控件，也不是完整 CSS Grid。

适合：

- 表单。
- 属性页。
- 设置面板。
- Dialog 内容区。
- label/input 多列对齐。
- 固定列 + 弹性列组合。

### 15.1 Track Types

第一版建议支持：

```text
px
auto
star
minmax
```

后续可扩展：

```text
content
percent
```

含义：

- `px`: 固定大小。
- `auto`: 根据 child preferred size 协商。
- `star`: 按权重分配剩余空间，例如 `1*`、`2*`。
- `minmax`: 对 track 应用最小/最大约束。
- `content`: 更明确的 intrinsic content size。
- `percent`: 按父可用空间比例。

### 15.2 Child Participation

```text
row
column
rowSpan
columnSpan
margin
alignX / alignY
```

### 15.3 Algorithm Boundary

第一版算法保持三轮：

1. 固定 track 先占位，非 span child 贡献 min/preferred 到 auto/content track。
2. percent 和 star 分配剩余空间，并应用 min/max。
3. span child 如果当前 span 总尺寸不足，再把差额分摊给可增长 track。

明确不做：

- CSS Grid 完整 auto placement。
- dense packing。
- grid-template-area。
- CSS min-content/max-content 全规范。
- 子元素跨 formatting context 反向影响父级复杂 reflow。

## 16. Flow Layout

`flow` 补齐 HTML 擅长的文档布局能力：div/span 式文档流、自动换行、图文和控件混排。

`flow` 是核心布局算法和轻量 layout mode，但普通 widget 的 `flow` 不自动创建 scrollbar。

适合：

- 帮助文档。
- 富文本说明。
- 设置页说明文字 + inline link/button。
- 自动换行标签。
- 图文混排。
- 聊天气泡内容。
- 简短文章/教程/README 类界面。

### 16.1 Display Participation

`flow` 中 child 可声明：

```text
display: block
display: inline
display: inline-block
display: none
display: absolute
```

含义：

- `block`: 类似 div，独占一行/一段，前后断行。
- `inline`: 类似 span，参与行内排版，可换行。
- `inline-block`: 作为一个矩形控件参与行内排版。
- `none`: 不参与布局。
- `absolute`: 脱离 flow，按 absolute 规则定位。

### 16.2 Supported Features

第一版支持：

- block stacking。
- inline line box。
- soft wrap。
- baseline。
- lineHeight。
- paragraphGap。
- textAlign。
- verticalAlign。
- inline margin。
- block margin。
- inline widget placeholder。
- content size reporting。

### 16.3 Explicit Non-Goals

第一版不做：

- float。
- margin collapse。
- complete CSS inline formatting context。
- position sticky/fixed。
- multi-column layout。
- browser-level CSS layout interactions。

### 16.4 Flow and Viewport

普通 `flow` layout 只负责排版和 content size reporting。

可滚动文档体验由标准控件提供：

```text
FlowPanel       flow layout only, no scrolling
FlowView        viewport + flow layout + scrollbar
DocumentView    FlowView plus document behavior
```

`FlowView` / `DocumentView` 负责：

- viewport。
- scroll offset。
- scrollbar mode。
- content size。
- clip。
- wheel/touch scroll。
- visible range optimization。
- flow tile/cache。
- anchor scroll into view。

## 17. Layout and Cache Interaction

布局系统必须与 XUI cache/damage 系统配合：

- layout bounds 变化触发 old rect + new rect damage。
- content size 变化触发 parent measure dirty 或 viewport range update。
- baseline 变化可能触发 row/table/flow 重新 arrange。
- overflow bounds 变化可能触发 cache bounds 变化。
- table/flow 的部分 dirty 应能映射到 cache tile。

布局 dirty 和 cache dirty 不应混为一类：

```text
measure dirty
arrange dirty
paint/cache dirty
damage dirty
```

## 18. Dynamic Style Interaction

样式变化对布局的影响必须分类：

- color/background/border color: paint/cache dirty only。
- font/text/lineHeight: measure dirty + cache dirty。
- padding/margin/gap: measure/arrange dirty。
- display/layout/position/dock/grid/table/flow participation: parent arrange dirty。
- shadow/focus ring/visual outsets: overflow/cache bounds dirty。

动态换肤时不应全树无差别 layout。样式系统应根据 style dependency 广播到受影响 widget。

## 19. Proposed API Shape

示意命名：

```c
typedef enum xui_layout_t {
    XUI_LAYOUT_ABSOLUTE,
    XUI_LAYOUT_STACK,
    XUI_LAYOUT_ROW,
    XUI_LAYOUT_COLUMN,
    XUI_LAYOUT_DOCK,
    XUI_LAYOUT_GRID,
    XUI_LAYOUT_TABLE,
    XUI_LAYOUT_FLOW
} xui_layout_t;
```

Flow display:

```c
typedef enum xui_display_t {
    XUI_DISPLAY_DEFAULT,
    XUI_DISPLAY_NONE,
    XUI_DISPLAY_BLOCK,
    XUI_DISPLAY_INLINE,
    XUI_DISPLAY_INLINE_BLOCK,
    XUI_DISPLAY_ABSOLUTE
} xui_display_t;
```

Table track:

```c
typedef enum xui_track_unit_t {
    XUI_TRACK_PX,
    XUI_TRACK_AUTO,
    XUI_TRACK_STAR,
    XUI_TRACK_MINMAX,
    XUI_TRACK_CONTENT,
    XUI_TRACK_PERCENT
} xui_track_unit_t;
```

## 20. Design Decisions So Far

- 现有 XUI 的 APP 布局心智基本正确，不需要推倒成浏览器式布局。
- `row` 和 `column` 作为公开一等布局保留。
- `flow` 是缺失的核心能力，用于文档流和 div/span 式混排。
- `flow` 子元素的 display 属性只在 `flow` formatting context 中生效。
- 普通 widget 的 `flow` 不自动提供 viewport 和 scrollbar。
- `FlowView` / `DocumentView` 作为标准控件组合 flow + viewport。
- `grid` 保持轻量固定网格。
- 新增 `table` 做二维大小协商，能力边界控制在 WPF Grid / CSS Grid 核心子集。
- 控件级布局能力不进入核心 layout enum。
- 不适用于当前 layout context 的属性 debug warning，release ignored。

## 21. Open Questions

- `table` 第一版是否支持 `percent` 和 `content`，还是先只做 `px/auto/star/minmax`。
- `flow` 第一版是否内建文本 run，还是只把 text 作为特殊 inline widget。
- Baseline API 是所有 widget 都可返回，还是只有文本类控件返回。
- `display: absolute` 在 flow 中是否第一版支持，还是后置。
- `grid` 是否保留当前 `columnSpan`，是否增加 `rowSpan`。
- `FlowView` 是核心控件还是可选标准控件模块。


