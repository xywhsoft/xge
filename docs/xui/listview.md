# XUI ListView

`ListView` 是基础列表控件，用于显示固定高度的文本行或自定义绘制行。新实现基于 `ScrollModel + ScrollFrame`，不再依赖旧 `VirtualViewBase`、slot widget 或控件内部手写滚动条。

## 职责边界

- `ScrollModel` 保存 viewport、content size 和滚动偏移。
- `ScrollFrame` 负责 viewport 裁剪、滚轮、横纵滚动条、滚动条拖拽和滚动条模式。
- `ListView` 负责 item 数据、启用状态、hover、focus index、单选/多选/范围选择、键盘导航和行绘制。

ListView 默认只启用纵向滚动条，横向滚动条隐藏。普通文本行会裁剪到 viewport 内；自定义行渲染器也会在 viewport clip 下执行。

## C API

- `xgeXuiListViewInit(list, context, widget)` 初始化列表。
- `xgeXuiListViewSetItems(list, items, count)` 设置文本项。
- `xgeXuiListViewSetEnabledItems(list, enabled, count)` 设置行启用状态。
- `xgeXuiListViewSetItemHeight(list, height)` 设置固定行高。
- `xgeXuiListViewSetSelected(list, index)` 设置单个当前项，并在布局可用后滚动到可见区域。
- `xgeXuiListViewSetSelectionMode(list, mode)` 支持 `single`、`multi`、`range`。
- `xgeXuiListViewSetSelectionBuffer(list, buffer, count)` 为多选/范围选择提供外部选择缓冲。
- `xgeXuiListViewSetItemRenderer(list, proc, user)` 自定义行绘制。
- `xgeXuiListViewSetScrollbarMode(list, mode)` 在完整滚动条和精简滚动条之间切换。
- `xgeXuiListViewSetColors(list, background, row, selected, text, bar, thumb)` 设置基础配色。

## Selection

- `single`：普通单选，点击新行会清空旧选择。
- `multi`：Ctrl 点击切换单行选中状态。
- `range`：Shift 点击按 anchor 选择连续范围，Ctrl 点击切换单行选中状态。

`multi` 和 `range` 需要通过 `xgeXuiListViewSetSelectionBuffer` 提供选择缓冲。键盘上下、PageUp/PageDown、Home/End 会移动当前项并保持可见。

## XSON

`listView` 已恢复 XSON 支持，常用字段：

- `items`
- `enabledItems`
- `itemHeight`
- `selected`
- `selectedItems`
- `selectionMode`
- `scrollY`
- `scrollbarMode`
- `backgroundColor` / `background`
- `rowColor` / `color`
- `selectedColor`
- `textColor`
- `disabledTextColor`
- `barColor`
- `thumbColor`

## 范例

- `examples/xui_listview`
- `examples/xui_listview_xson`
