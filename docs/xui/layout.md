# XUI Layout

XUI 布局系统由 Widget 提供。布局的输入是父节点 `contentRect`、子节点尺寸声明、margin、align、justify、grid/dock 参数和可选的 measure/layout 回调；输出是每个 Widget 的 `rect`、`outerRect`、`borderRect`、`paddingRect` 和 `contentRect`。

控件重构时要优先依赖布局系统，不要在控件内部写一次性的排版逻辑。控件只在自己的 `contentRect` 内组织内容，例如 Button 的图标与文字混排；控件之间的排列、间距、拉伸和停靠都应由 Widget 布局完成。

## 基础概念

Widget 的盒模型从外到内为：

- `outerRect`: `borderRect` 加上 margin。
- `borderRect`: 背景、边框和控件本体区域。
- `paddingRect`: `borderRect` 扣除 border。
- `contentRect`: `paddingRect` 扣除 padding，布局子元素和控件内容的主要区域。

布局总是以父节点的 `contentRect` 作为子节点可用区域。子节点的 margin 会占用父布局空间；子节点自己的 padding 和 border 会影响自己的 `contentRect`，但不直接影响兄弟节点，除非其尺寸是 `content`。

## 尺寸单位

```c
xgeXuiWidgetSetSize(widget, xgeXuiSizePx(120), xgeXuiSizePx(40));
xgeXuiWidgetSetSize(widget, xgeXuiSizePercent(100), xgeXuiSizeContent());
xgeXuiWidgetSetSize(widget, xgeXuiSizeGrow(1), xgeXuiSizePx(32));
```

| 单位 | XSON | 说明 |
| --- | --- | --- |
| `XGE_XUI_SIZE_PX` | `120` / `"120px"` | 固定像素。 |
| `XGE_XUI_SIZE_DIP` | `"120dip"` | 乘以 XUI DIP 缩放。 |
| `XGE_XUI_SIZE_PERCENT` | `"100%"` | 相对父节点可用尺寸。 |
| `XGE_XUI_SIZE_CONTENT` | `"content"` | 使用控件 measure 或子节点测量结果。 |
| `XGE_XUI_SIZE_GROW` | `"grow"` / `"grow:2"` | 在 row/column 中按权重分配剩余空间。 |

`minWidth`、`minHeight`、`maxWidth`、`maxHeight` 会在尺寸解析后生效。`max` 为 0 时表示不限制最大值。

`grow` 的权重小于等于 0 时按 1 处理。多个 grow 子节点会按权重分配剩余空间，并在 min/max 约束下冻结已触达约束的子节点，再把剩余空间继续分配给其它 grow 子节点。

## 通用布局属性

```c
xgeXuiWidgetSetLayout(parent, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetGap(parent, 8);
xgeXuiWidgetSetAlign(child, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_CENTER);
xgeXuiWidgetSetJustify(parent, XGE_XUI_JUSTIFY_START);
xgeXuiWidgetSetMarginPx(child, 4, 4, 4, 4);
xgeXuiWidgetSetPaddingPx(parent, 12, 8, 12, 8);
```

布局类型：

- `XGE_XUI_LAYOUT_ABSOLUTE`
- `XGE_XUI_LAYOUT_ROW`
- `XGE_XUI_LAYOUT_COLUMN`
- `XGE_XUI_LAYOUT_STACK`
- `XGE_XUI_LAYOUT_GRID`
- `XGE_XUI_LAYOUT_DOCK`

对齐：

- `XGE_XUI_ALIGN_START`
- `XGE_XUI_ALIGN_CENTER`
- `XGE_XUI_ALIGN_END`
- `XGE_XUI_ALIGN_STRETCH`

主轴分布：

- `XGE_XUI_JUSTIFY_START`
- `XGE_XUI_JUSTIFY_CENTER`
- `XGE_XUI_JUSTIFY_END`
- `XGE_XUI_JUSTIFY_SPACE_BETWEEN`

`align` 是子节点属性，决定子节点在父布局分配给它的 slot 内如何摆放。`justify` 是父节点属性，只影响 row/column 的主轴剩余空间分布；当存在 grow 子节点时，剩余空间被 grow 消耗，justify 不再移动整体。

## Absolute

Absolute 使用子节点的 `localRect`、anchor 和 margin 布局。适合固定位置、弹层内部手工定位、游戏 HUD 等场景。

```c
xgeXuiWidgetSetLayout(parent, XGE_XUI_LAYOUT_ABSOLUTE);
xgeXuiWidgetSetRect(child, (xge_rect_t){ 20, 16, 120, 36 });
xgeXuiWidgetSetAnchorPx(child, XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_TOP, 0, 12, 20, 0);
```

规则：

- 默认使用 `localRect.x/y/w/h`。
- 设置 left/right 两侧 anchor 时，宽度由父内容区域扣除左右 anchor 和 margin 得出。
- 只设置 right 时，子节点贴父内容区域右侧。
- top/bottom 同理。
- 子节点最终仍会经过 `alignX/alignY` 调整。

XSON：

```json
{
  "type": "absolute",
  "children": [
    { "type": "button", "x": 20, "y": 16, "width": 120, "height": 36 },
    { "type": "button", "anchor": "right top", "right": 20, "top": 12, "width": 120, "height": 36 }
  ]
}
```

## Row

Row 水平排列子节点，主轴为 X，交叉轴为 Y。

```c
xgeXuiWidgetSetLayout(row, XGE_XUI_LAYOUT_ROW);
xgeXuiWidgetSetGap(row, 8);
xgeXuiWidgetSetJustify(row, XGE_XUI_JUSTIFY_SPACE_BETWEEN);

xgeXuiWidgetSetSize(left, xgeXuiSizePx(80), xgeXuiSizePx(32));
xgeXuiWidgetSetSize(center, xgeXuiSizeGrow(1), xgeXuiSizePx(32));
xgeXuiWidgetSetSize(right, xgeXuiSizePx(80), xgeXuiSizePx(32));
```

规则：

- 固定宽度、百分比宽度、content 宽度先占用空间。
- `gap` 在相邻子节点之间占用空间。
- `width: grow` 的子节点按权重瓜分剩余宽度。
- 子节点高度由自己的 height 决定；如果 `alignY == stretch` 且 height 是 `content` 或 `grow`，则拉伸到可用高度。
- 没有 grow 子节点时，`justify` 决定剩余水平空间放在前面、中间、后面或节点之间。

常见用法：

- 表单一行：左侧 Label 固定宽度，右侧输入框 grow。
- 工具栏：按钮固定宽度，弹性 spacer 使用 grow。
- 状态栏：多个固定项和一个 grow 空白区。

XSON：

```json
{
  "type": "row",
  "gap": 8,
  "alignY": "center",
  "children": [
    { "type": "label", "width": 80, "height": 28, "text": "Name" },
    { "type": "input", "width": "grow", "height": 28 },
    { "type": "button", "width": 72, "height": 28, "text": "OK" }
  ]
}
```

## Column

Column 垂直排列子节点，主轴为 Y，交叉轴为 X。

```c
xgeXuiWidgetSetLayout(column, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetGap(column, 10);

xgeXuiWidgetSetSize(header, xgeXuiSizePercent(100), xgeXuiSizePx(44));
xgeXuiWidgetSetSize(body, xgeXuiSizePercent(100), xgeXuiSizeGrow(1));
xgeXuiWidgetSetSize(footer, xgeXuiSizePercent(100), xgeXuiSizePx(36));
```

规则：

- 固定高度、百分比高度、content 高度先占用空间。
- `gap` 在相邻子节点之间占用空间。
- `height: grow` 的子节点按权重瓜分剩余高度。
- 子节点宽度由自己的 width 决定；如果 `alignX == stretch` 且 width 是 `content` 或 `grow`，则拉伸到可用宽度。
- 没有 grow 子节点时，`justify` 决定剩余垂直空间分布。

Column 是 XUI 范例和表单最常用的布局。控件列表、属性面板、设置页优先使用 Column，而不是 absolute 手工摆放。

## Stack

Stack 把所有子节点放进同一个父内容区域，并按子节点顺序叠放。

```c
xgeXuiWidgetSetLayout(stack, XGE_XUI_LAYOUT_STACK);
xgeXuiWidgetSetAlign(background, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
xgeXuiWidgetSetAlign(badge, XGE_XUI_ALIGN_END, XGE_XUI_ALIGN_START);
```

规则：

- 每个子节点都获得父 `contentRect` 作为 slot。
- margin 会缩小 slot。
- `alignX/alignY` 决定子节点在 slot 内的位置。
- `stretch` 可以把 content/grow 尺寸拉满 slot。

适用场景：

- 背景层 + 内容层。
- 一个区域右上角叠加 Badge、状态标识或关闭按钮。
- 简单遮罩层。

## Grid

Grid 使用固定列数和行高排列子节点。

```c
xgeXuiWidgetSetLayout(grid, XGE_XUI_LAYOUT_GRID);
xgeXuiWidgetSetGrid(grid, 3, 64.0f, 8.0f, 8.0f);
xgeXuiWidgetSetGridColumnSpan(item, 2);
```

规则：

- 列数最小为 1。
- 单元格宽度 = `(contentWidth - columnGap * (columns - 1)) / columns`。
- `rowHeight > 0` 时使用固定行高。
- `rowHeight <= 0` 时，行高等于单元格宽度，适合做方形网格。
- 子节点按顺序填充；如果当前行剩余列数不足 `columnSpan`，会换到下一行。
- `columnSpan` 不能超过列数。
- 子节点 margin 会缩小单元格内的 slot。
- align/stretch 规则与 Stack 类似。

XSON：

```json
{
  "type": "grid",
  "columns": 3,
  "rowHeight": 64,
  "columnGap": 8,
  "rowGap": 8,
  "children": [
    { "type": "button", "text": "A" },
    { "type": "button", "text": "B", "columnSpan": 2 },
    { "type": "button", "text": "C" }
  ]
}
```

也可以写成：

```json
{
  "type": "grid",
  "grid": {
    "columns": 3,
    "rowHeight": 64,
    "columnGap": 8,
    "rowGap": 8
  }
}
```

Grid 适合图标墙、按钮矩阵、范例展示区和固定格式表单，不适合需要复杂跨行布局的场景。

## Dock

Dock 按子节点顺序逐个消耗父内容区域的剩余矩形。

```c
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_DOCK);

xgeXuiWidgetSetDock(header, XGE_XUI_DOCK_TOP);
xgeXuiWidgetSetSize(header, xgeXuiSizeGrow(1), xgeXuiSizePx(48));

xgeXuiWidgetSetDock(sidebar, XGE_XUI_DOCK_LEFT);
xgeXuiWidgetSetSize(sidebar, xgeXuiSizePx(220), xgeXuiSizeGrow(1));

xgeXuiWidgetSetDock(content, XGE_XUI_DOCK_FILL);
xgeXuiWidgetSetSize(content, xgeXuiSizeGrow(1), xgeXuiSizeGrow(1));
```

Dock 值：

- `XGE_XUI_DOCK_LEFT`
- `XGE_XUI_DOCK_TOP`
- `XGE_XUI_DOCK_RIGHT`
- `XGE_XUI_DOCK_BOTTOM`
- `XGE_XUI_DOCK_FILL`
- `XGE_XUI_DOCK_CENTER` 等同于 fill。

规则：

- 子节点顺序很重要。先添加的 dock 节点先占用空间。
- top/bottom 消耗高度，left/right 消耗宽度。
- fill/center 使用当前剩余区域，但不会继续消耗剩余区域给后续节点预留空间。
- `gap` 会随每次消耗一起扣除。
- margin 会缩小当前子节点 slot。
- dock top/bottom 时，`alignX: stretch` 可以拉满宽度；dock left/right 时，`alignY: stretch` 可以拉满高度。

XSON：

```json
{
  "type": "dock",
  "children": [
    { "type": "panel", "dock": "top", "height": 48 },
    { "type": "panel", "dock": "left", "width": 220 },
    { "type": "panel", "dock": "fill" }
  ]
}
```

Dock 适合应用主框架，例如标题栏、侧边栏、状态栏和主内容区域。

## Measure

当 Widget 尺寸是 `content` 时，布局系统会调用 measure：

```c
xgeXuiWidgetSetMeasure(widget, MeasureProc);
xgeXuiWidgetSetMeasureUser(widget, MeasureProc, user);
```

如果控件没有 measure 回调，Widget 会根据子节点测量。控件 measure 的结果会与子节点测量结果取较大值，再加上 padding 和 border。

控件重构要求：

- Label、Image、Separator、Progress、Button 等控件需要提供合理的 measure proc。
- `content` 尺寸必须语义明确，例如 Label 返回文字尺寸，Image 返回源图尺寸，Separator 返回线条最小尺寸。
- measure 不应该产生副作用，不应该修改树结构或触发绘制。

## 自定义 Layout Proc

复杂控件可以接管子节点布局：

```c
xgeXuiWidgetSetLayoutProc(widget, LayoutProc, user);
```

如果设置了 `procLayout`，Widget 不再执行内置 absolute/row/column/stack/grid/dock 布局，而是调用自定义布局过程。自定义布局仍应使用 Widget 的盒模型结果，通常基于 `widget->tContentRect` 安排子节点。

只有复合控件才应使用自定义 layout proc，例如 SplitLayout、VirtualList、ScrollView、TableView 等。普通页面布局不要用自定义 layout proc。

## Dirty 与更新流程

布局更新发生在 `xgeXuiUpdate` 中：

1. 更新 Widget 树。
2. 从 root 开始执行布局。
3. 更新 overlay root。
4. 更新 tooltip。

会触发布局脏标记的操作：

- 修改 size/min/max。
- 修改 margin/padding。
- 修改 layout/dock/grid/gap/align/justify。
- 修改 rect/anchor。
- 添加、移除、重排子节点。
- 修改会影响 measure 的控件内容。

API：

```c
xgeXuiWidgetMarkLayout(widget);
xgeXuiWidgetMarkPaint(widget);
xgeXuiWidgetMarkStyle(widget);
```

批量更新：

```c
xgeXuiLayoutBatchBegin(&xui);
/* 多次修改布局相关属性 */
xgeXuiLayoutBatchEnd(&xui);
```

批量期间的 layout/paint 脏标记会合并，结束时统一请求刷新。

## XSON 通用布局字段

```json
{
  "type": "column",
  "width": "100%",
  "height": "100%",
  "minWidth": 320,
  "maxWidth": 960,
  "padding": [16, 12, 16, 12],
  "margin": [0, 0, 0, 0],
  "gap": 8,
  "alignX": "stretch",
  "alignY": "start",
  "justify": "start",
  "children": []
}
```

尺寸字段：

- `width`
- `height`
- `minWidth`
- `minHeight`
- `maxWidth`
- `maxHeight`

边距字段：

- `margin`: `[left, top, right, bottom]` 或对象。
- `padding`: `[left, top, right, bottom]` 或对象。

布局字段：

- `layout`: 覆盖节点默认布局。
- `gap`
- `align`: 同时设置 X/Y。
- `alignX`
- `alignY`
- `justify`
- `dock`
- `grid`
- `columns` / `gridColumns`
- `rowHeight`
- `columnGap`
- `rowGap`
- `columnSpan` / `gridColumnSpan`

类型到默认布局：

- `absolute` / `panel`: absolute
- `row`: row
- `column`: column
- `stack`: stack
- `grid`: grid
- `dock`: dock
- 控件节点默认 absolute

## 设计建议

- 应用主框架优先使用 Dock。
- 表单、属性列表和纵向页面优先使用 Column。
- 工具条、表单行和状态栏优先使用 Row。
- 重复卡片、按钮矩阵、范例展示优先使用 Grid。
- 层叠徽标、遮罩、背景层优先使用 Stack。
- Absolute 只用于确实需要固定坐标或 anchor 的局部区域。

控件重构时要把布局职责放回 Widget：

- 控件不要暴露“外部排版”参数，例如控件之间的 gap、容器对齐、列表排列。
- 控件内部可以有内容布局参数，例如 Button 的 iconPlacement/iconGap，但这只影响按钮内部语义。
- 需要复杂自定义排版时，优先做复合控件或 layout proc，而不是让基础控件承担页面布局职责。
