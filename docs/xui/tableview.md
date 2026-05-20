# XUI TableView

`TableView` 是静态表格展示控件，也是后续 `TableGrid` 可编辑表格的基础设施层。它负责表格几何、表头、滚动、裁剪、选择、命中测试、样式、合并单元格和自定义单元格绘制；它不负责表单编辑、公式、范围选择、范围复制粘贴或撤销栈。

旧 TableView 实现已清理，新实现不保留旧 API 和旧行为兼容。后续代码、XSON 和范例都以本文为唯一设计口径。

## 职责边界

- `ScrollModel` 保存 content size、viewport size 和滚动偏移。
- `ScrollFrame` 负责 viewport、横纵滚动条、滚轮、滚动条拖拽、裁剪和滚动条模式。
- `TableView` 负责列模型、行模型、单元格模型、表头、布局测量、命中测试、选择、hover、列宽拖动、合并单元格和绘制。
- `TableGrid` 在 `TableView` 之上增加编辑器生命周期，不重复实现表格布局、滚动、命中和绘制基础逻辑。

`TableView` 不持有业务数据。控件保存结构和状态，单元格值通过外部 adapter 或回调按需读取。这样可以支撑大数据、虚拟数据和业务模型绑定。

## 非目标

- 不做电子表格。
- 不支持公式。
- 不支持 Excel 式范围选择。
- 不支持范围复制粘贴。
- 不内建撤销栈。
- 第一版不做多级表头。
- 第一版不做冻结行列；若后续补冻结，合并单元格不能跨冻结边界。

## 默认值

- 默认列宽：`80px`。
- 默认行高：`20px`。
- 默认表头高度：`24px`。
- 默认滚动条模式：`compact`。
- 默认选择模式：单元格选择。
- 默认横向和纵向滚动条策略：`auto`。

## 数据模型

### Column

列定义字段：

- `id`：列 ID，优先作为业务字段名和 XSON 绑定键。
- `title`：表头文本。
- `width`：当前列宽，默认 `80`。
- `minWidth`：最小列宽。
- `maxWidth`：最大列宽。
- `visible`：是否显示。
- `resizable`：是否允许拖动改宽。
- `align`：单元格内容对齐。
- `type`：列默认单元格类型，供格式化、XSON 和后续 `TableGrid` 编辑器映射使用。
- `formatter`：值到展示文本的格式化回调。
- `renderer`：列级自定义单元格渲染回调。
- `style`：列级样式，目前参与背景色和网格/边框色合并。

### Row

行定义字段：

- `height`：行高，默认使用表格默认行高。
- `selected`：行选择状态。
- `disabled`：禁用行，禁用行不响应选择。
- `style`：行级样式。

### Cell

单元格状态由 adapter 提供或由 TableView 状态层补充：

- `value`：业务值，TableView 本身不持有。
- `text`：展示文本，可由 formatter 生成。
- `rowSpan` / `colSpan`：合并单元格跨度。
- `disabled`：禁用单元格。
- `style`：单元格样式。
- `tooltip`：单元格动态 tooltip 文本，由 TableView 挂到 body widget 的 tooltip resolver 自动解析。
- `editing` / `invalid` / `dirty`：为后续 TableGrid 预留的状态标记，也会进入 renderer state flags。
- `renderer`：单元格级自定义渲染回调。

合并单元格只允许主格定义 `rowSpan / colSpan`。被覆盖的从属格不绘制、不响应普通单元格命中，命中测试应返回主格坐标。

第一版合并规则：

- 允许普通区域内的行列合并。
- 不允许跨隐藏列。
- 不允许跨未来的冻结边界。
- 不允许越过 adapter 当前声明的行列范围。
- 合并区域参与裁剪，不能绘制到 viewport 或滚动条区域外。

## 选择模型

`TableView` 支持两种互斥选择模式：

- `row`：只能选择整行。
- `cell`：只能选择单元格。

不支持范围选择。两种模式都支持 hover。禁用行或禁用单元格不允许成为选择目标。

建议交互：

- 单击行或单元格设置选择。
- 鼠标移动更新 hover。
- `Up` / `Down` / `Left` / `Right` 移动选择。
- `PageUp` / `PageDown` 按可见范围跳转。
- `Home` / `End` 跳转到行首/行尾或表格首尾。
- 选择移动后调用 `ensureVisible` 保证目标可见。

## 表头

第一版表头能力：

- 单级表头。
- 表头背景、文字、分隔线绘制。
- 表头自定义渲染。
- 列宽拖动。
- 点击排序事件，但排序动作由业务层执行。

列宽拖动只改变列模型的 width，并重新计算 content width。拖动时不修改业务数据。

## 样式

样式优先级固定为：

```text
theme < table < column < row < cell < state < custom renderer
```

基础样式字段：

- `backgroundColor`
- `headerColor`
- `headerTextColor`
- `rowColor`
- `alternateRowColor`
- `hoverColor`
- `selectedColor`
- `disabledColor`
- `gridColor`
- `textColor`
- `disabledTextColor`
- `focusRingColor`

状态样式包括 hover、selected、focused、disabled、editing、invalid、dirty、header-hover、header-dragging 等。自定义 renderer 返回已处理时，默认单元格内容绘制不再执行；TableView 仍负责裁剪、滚动、命中和选择状态。

当前实现中，`column / row / cell` 的 `xge_xui_style_t` 主要参与 `backgroundColor` 和 `borderColor` 合并；文本颜色仍使用 TableView 的 `textColor / disabledTextColor`。后续如需要更细粒度文本样式，可在专门的 TableView style snapshot 中扩展，而不直接污染 Widget 通用 style。

## 自定义渲染

TableView 提供三层自定义渲染：

- 表头 renderer：接管指定列的 header 绘制。
- 列 renderer：接管该列所有单元格绘制。
- 单元格 renderer：接管指定单元格绘制。
- 列 formatter：当 cell 只提供 `value` 而不提供 `text` 时，将业务值格式化为展示文本。

renderer 接收：

- 当前 widget。
- row、column。
- screen-space cell rect。
- value/text。
- state flags。
- user data。

renderer 返回非 0 表示已处理内容绘制；TableView 仍绘制默认背景、网格线、focus/invalid/editing 标记和滚动裁剪。也就是说，自定义 renderer 主要接管内容区域，而不是接管整个表格框架。

## 布局与滚动

TableView 直接持有 `ScrollModel + ScrollFrame`，不创建每个单元格 widget。

布局步骤：

1. 根据 widget content rect 计算表格外框。
2. 计算表头 rect 和 body viewport rect。
3. 根据可见列、列宽和 row height 计算 content width/height。
4. 将 body content size 写入 ScrollFrame。
5. 由 ScrollFrame 决定滚动条显示、viewport rect 和滚动偏移。
6. 根据 offset 计算可见行列范围。

横向滚动只影响表格 body 和表头列内容，表头自身不随纵向滚动移动。纵向滚动只影响 body 行。

## 命中测试

命中结果需要区分：

- `none`
- `header`
- `headerResize`
- `row`
- `cell`
- `mergedCell`
- `scrollbar`

命中测试必须基于 ScrollFrame 的 viewport 和 ScrollModel 坐标转换，不允许控件内部重复写一套滚动偏移算法。

## TableGrid 关系

`TableGrid` 复用 `TableView` 的表格基础能力，只增加编辑层。

TableGrid 的编辑器类型口径：

- `text` -> `Input`
- `int` / `float` -> `NumericInput`
- `bool` -> `CheckBox` 或 `Toggle`
- `textarea` -> 只读 `Input + v 按钮`，弹出带 `TextEdit` 的 popup，input 显示摘要。
- `date` / `time` / `datetime` -> `DatePicker`
- `enum` -> `ComboBox`
- `color` -> `ColorPicker`
- `picker` -> `Input + ... 按钮 + callback`
- `custom` -> 用户自定义编辑控件和事件逻辑。

TableGrid 当前支持两种轻量激活模式：

- `display`：默认按 TableView 绘制，双击或键盘确认创建对应编辑器。
- `quick`：单击单元格后立即创建对应编辑器。

真正的 `immediate` 模式，即可见区域直接渲染表单控件，属于后续 TableGrid 扩展任务，不能通过为每个单元格创建 widget 的方式实现。

TableGrid 的完整 API 和跟踪项见 [tablegrid.md](tablegrid.md) 与 [tablegrid-spec.md](tablegrid-spec.md)。

## C API 目标口径

下面是目标 API 形态，具体命名以实现时的 `xge.h` 为准，但不保留旧 TableView API 兼容。

```c
int xgeXuiTableViewInit(xge_xui_table_view table, xge_xui_context context, xge_xui_widget widget);
void xgeXuiTableViewUnit(xge_xui_table_view table);

void xgeXuiTableViewSetColumns(xge_xui_table_view table, const xge_xui_table_view_column_t* columns, int count);
void xgeXuiTableViewSetRows(xge_xui_table_view table, const xge_xui_table_view_row_t* rows, int count);
void xgeXuiTableViewSetAdapter(xge_xui_table_view table, xge_xui_table_view_count_proc count, xge_xui_table_view_cell_proc cell, void* user);

void xgeXuiTableViewSetDefaultMetrics(xge_xui_table_view table, float columnWidth, float rowHeight, float headerHeight);
void xgeXuiTableViewSetSelectionMode(xge_xui_table_view table, int mode);
void xgeXuiTableViewSetSelectedRow(xge_xui_table_view table, int row);
void xgeXuiTableViewSetSelectedCell(xge_xui_table_view table, int row, int column);
void xgeXuiTableViewGetSelectedCell(xge_xui_table_view table, int* row, int* column);

void xgeXuiTableViewSetMergeProvider(xge_xui_table_view table, xge_xui_table_view_merge_proc proc, void* user);
void xgeXuiTableViewSetHeaderRenderer(xge_xui_table_view table, xge_xui_table_view_header_renderer_proc proc, void* user);
void xgeXuiTableViewSetCellRenderer(xge_xui_table_view table, xge_xui_table_view_cell_renderer_proc proc, void* user);
void xgeXuiTableViewSetColumnFormatter(xge_xui_table_view table, int column, xge_xui_table_view_format_proc proc, void* user);
void xgeXuiTableViewSetSort(xge_xui_table_view table, xge_xui_table_view_sort_proc proc, void* user);
void xgeXuiTableViewSetSelect(xge_xui_table_view table, xge_xui_table_view_select_proc proc, void* user);
void xgeXuiTableViewSetHover(xge_xui_table_view table, xge_xui_table_view_hover_proc proc, void* user);

void xgeXuiTableViewSetScrollbarMode(xge_xui_table_view table, int mode);
void xgeXuiTableViewSetColors(xge_xui_table_view table, uint32_t background, uint32_t header, uint32_t row, uint32_t selected, uint32_t grid, uint32_t text);
void xgeXuiTableViewSetOffset(xge_xui_table_view table, float scrollX, float scrollY);
void xgeXuiTableViewGetOffset(xge_xui_table_view table, float* scrollX, float* scrollY);
void xgeXuiTableViewGetActiveCell(xge_xui_table_view table, int* row, int* column);
int xgeXuiTableViewGetCellContentRect(xge_xui_table_view table, int row, int column, xge_rect_t* rect);
int xgeXuiTableViewGetCellRect(xge_xui_table_view table, int row, int column, xge_rect_t* rect);
xge_xui_widget xgeXuiTableViewGetBodyWidget(xge_xui_table_view table);
xge_xui_widget xgeXuiTableViewGetViewportWidget(xge_xui_table_view table);
void xgeXuiTableViewRefresh(xge_xui_table_view table);
void xgeXuiTableViewEnsureCellVisible(xge_xui_table_view table, int row, int column);
```

## XSON

`tableView` 恢复时使用新字段口径：

```json
{
  "type": "tableView",
  "defaultColumnWidth": 80,
  "defaultRowHeight": 20,
  "headerHeight": 24,
  "selectionMode": "cell",
  "scrollbarMode": "compact",
  "columns": [
    { "id": "name", "title": "Name", "width": 160, "resizable": true, "align": "left", "type": "text" },
    { "id": "status", "title": "Status", "width": 90, "align": "center", "type": "enum" }
  ],
  "rows": [
    { "height": 24, "selected": false },
    { "height": 24, "disabled": true }
  ],
  "cells": [
    ["Alpha", { "text": "Ready", "tooltip": "State from XSON" }],
    ["Beta", { "text": "Disabled", "disabled": true, "invalid": true }]
  ],
  "merges": [
    { "row": 0, "column": 0, "rowSpan": 1, "colSpan": 2 }
  ]
}
```

XSON 的 `cells` 只用于范例和小型静态数据。大数据和业务数据应通过 C adapter 或后续绑定系统提供。

## 范例

TableView 范例成对提供：

- `examples/xui_tableview`
- `examples/xui_tableview_xson`

范例需要覆盖：

- 默认列宽和默认行高。
- 行选择模式。
- 单元格选择模式。
- 列宽拖动。
- 横向和纵向滚动。
- 自定义表头渲染。
- 自定义单元格渲染。
- 合并单元格。
- 禁用行/单元格。
- hover、selected、focused 状态。
- compact/full 两种滚动条显示。
