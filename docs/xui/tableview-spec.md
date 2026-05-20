# XUI TableView Spec

本 spec 用于跟踪 `TableView` 重构。`TableView` 是静态表格控件，同时是后续 `TableGrid` 的基础设施层。本轮不维护旧 TableView 兼容。

## 设计状态

- [x] 明确拆分 `TableView` 和 `TableGrid`。
- [x] 明确 `TableView` 不持有业务数据，只持有结构、状态、样式和缓存。
- [x] 明确默认列宽 `80px`、默认行高 `20px`。
- [x] 明确选择模式为 `row` 或 `cell` 二选一，不支持范围选择。
- [x] 明确第一版支持单级表头、列宽拖动、点击排序事件和表头自定义渲染。
- [x] 明确单元格合并规则：不跨隐藏列、不跨未来冻结边界、不越界。
- [x] 明确样式优先级：`theme < table < column < row < cell < state < custom renderer`。
- [x] 明确 `TableGrid` 只在 `TableView` 上增加编辑层。
- [x] 明确 `textarea` 编辑器是普通单元格摘要 + TextEdit popup。
- [x] 明确 `picker` 是正式编辑器类型名，形态为普通单元格文本 + `...` 操作按钮 + callback。
- [x] 新增 `docs/xui/tableview.md`。
- [x] 新增 `docs/xui/tableview-spec.md`。

## 实现任务

- [x] 清理旧 TableView 结构体和 API 口径，不保留旧 `VirtualViewBase` / 手写滚动条路径。
- [x] 新增/重建 TableView 数据结构：table、column、row、cell、style、colors、hit-test、merge、adapter。
- [x] 接入 `ScrollModel + ScrollFrame`，body 区域不手写滚动条和滚动偏移算法。
- [x] 实现默认 metrics：column width、row height、header height。
- [x] 实现列模型：id、title、width、minWidth、maxWidth、visible、resizable、align、type、formatter、renderer、style。
- [x] 实现行模型：height、selected、disabled、style。
- [x] 实现外部数据 adapter，支持按 row/column 请求 value/text/style/disabled/merge。
- [x] 实现 content size 测量，支持横向和纵向滚动。
- [x] 实现可见行列范围计算，绘制时只处理可见区域。
- [x] 实现表头绘制。
- [x] 实现表头自定义 renderer。
- [x] 实现列宽拖动，拖动时 clamp 到 min/max。
- [x] 实现点击表头排序事件，排序动作交给业务层。
- [x] 实现基础单元格绘制：背景、文本、网格线、禁用态。
- [x] 实现样式优先级合并。
- [x] 实现 hover 状态。
- [x] 实现 row selection 模式。
- [x] 实现 cell selection 模式。
- [x] 实现键盘导航：方向键、PageUp/PageDown、Home/End。
- [x] 实现 `ensureCellVisible`。
- [x] 实现合并单元格测量、绘制和命中。
- [x] 实现被合并从属格跳过绘制和命中转主格。
- [x] 实现单元格自定义 renderer。
- [x] 实现事件回调：select、sort、columnResize、hoverChange。
- [x] 实现 compact/full 滚动条模式切换。
- [x] 实现 disabled row/cell 交互跳过。
- [x] 实现 focus ring 和 Widget 基础焦点接入。
- [x] 实现 tooltip 支持，默认走 Widget tooltip，单元格动态 tooltip 通过 resolver 接入。
- [x] 恢复 `tableView` XSON 类型，字段使用新口径。
- [x] XSON 对旧字段返回明确错误或忽略策略，不能走旧实现。
- [x] 实现过程中持续同步 `docs/xui/tableview.md`。
- [x] 新增 `examples/xui_tableview`。
- [x] 新增 `examples/xui_tableview_xson`。

## API 验收

- [x] C API 不暴露旧 `VirtualViewBase` 口径。
- [x] C API 能设置列、行、adapter、renderer、merge provider、selection mode、colors、metrics、scrollbar mode。
- [x] C API 能读取当前 row/cell 选择。
- [x] C API 能刷新 adapter 数据。
- [x] C API 能确保指定单元格可见。
- [x] 回调命名和参数能表达 row、column、rect、state、value/text、user data。
- [x] TableGrid 后续可以复用 TableView 的 hit-test、selection、layout、paint state 和 ensure visible 能力。

## XSON 验收

- [x] `tableView` 支持 `defaultColumnWidth`、`defaultRowHeight`、`headerHeight`。
- [x] `tableView` 支持 `selectionMode`：`row` / `cell`。
- [x] `tableView` 支持 `scrollbarMode`：`compact` / `full`。
- [x] `tableView` 支持 `columns` 对象数组。
- [x] `columns` 支持 `id`、`title`、`width`、`minWidth`、`maxWidth`、`visible`、`resizable`、`align`、`type`。
- [x] `tableView` 支持 `rows` 对象数组。
- [x] `rows` 支持 `height`、`selected`、`disabled`。
- [x] `tableView` 支持小型静态 `cells` 二维数组。
- [x] `tableView` 支持 `merges` 数组。
- [x] `tableView` 支持基础颜色字段。
- [x] `tableView` 加载错误能准确定位字段路径。

## 范例验收

- [x] `xui_tableview` 可编译运行。
- [x] `xui_tableview_xson` 可编译运行。
- [x] 范例覆盖默认 metrics。
- [x] 范例覆盖行选择模式。
- [x] 范例覆盖单元格选择模式。
- [x] 范例覆盖列宽拖动。
- [x] 范例覆盖横向滚动。
- [x] 范例覆盖纵向滚动。
- [x] 范例覆盖 compact/full 滚动条。
- [x] 范例覆盖自定义表头渲染。
- [x] 范例覆盖自定义单元格渲染。
- [x] 范例覆盖合并单元格。
- [x] 范例覆盖禁用行/单元格。
- [x] 范例覆盖 hover、selected、focused 状态。
- [x] 范例覆盖大数据 adapter，不为每个单元格创建 widget。

## 后续 TableGrid 衔接

- [x] TableView 暴露足够的 active cell 定位能力，供 TableGrid 定位编辑器。
- [x] TableView 暴露 cell rect 查询，返回 content/screen 坐标。
- [x] TableView 支持编辑器遮罩层或 overlay widget 挂载点。
- [x] TableView 的 renderer/state flags 能表达 editing/invalid/dirty 等未来状态。
- [x] TableView 的 data adapter 不假定 value 是字符串。

## 验收标准

- 内容不能绘制到滚动条区域。
- 表头和 body 横向滚动必须同步。
- 表头不能随纵向滚动移动。
- 文字和网格线必须裁剪在 viewport 内。
- 滚动条 thumb 拖拽必须跟手。
- 列宽拖动必须跟手，且不能产生负宽度或越过 min/max。
- 合并单元格命中必须返回主格。
- 自定义 renderer 不能破坏默认裁剪。
- 行选择和单元格选择不能同时生效。
- 禁用行/单元格不能被选中。
- XSON 范例和 C 范例必须成对存在。
