# XUI TreeView

`TreeView` 是层级数据控件，用于显示可展开/折叠的固定行高树。新实现基于 `ScrollModel + ScrollFrame`，不再依赖旧 `VirtualViewBase`、slot widget 或控件内部手写滚动条。

## 职责边界

- `ScrollModel` 保存 viewport、content size 和纵向滚动偏移。
- `ScrollFrame` 负责 viewport 裁剪、滚轮、滚动条、滚动条拖拽和滚动条模式。
- `TreeView` 负责节点数据、可见节点展开表、启用状态、勾选状态、hover、focus、选择、键盘导航、节点装饰和行绘制。

TreeView 默认只启用纵向滚动条，横向滚动条隐藏。节点不会为每一行创建子 widget，行内容直接在 viewport clip 下绘制；自定义行渲染器也在同一裁剪区域中执行。

## 节点模型

- `id` 是节点唯一编号。
- `parent` 是父节点编号，根节点使用 `-1`。
- `expanded` 控制子节点是否可见。
- `enabled` 为 `false` 时不可选择、不可点击、键盘导航会跳过。
- `icon` 控制是否绘制内置文件夹/文件图标。
- `check` 控制是否保留 checkbox 区域。
- `checked` 会自动启用 checkbox，并设置勾选状态。

## C API

- `xgeXuiTreeViewInit(tree, context, widget)` 初始化树。
- `xgeXuiTreeViewClear(tree)` 清空节点。
- `xgeXuiTreeViewAddNode(tree, id, parentId, text)` 添加节点。
- `xgeXuiTreeViewSetAdapter(tree, countProc, nodeProc, user)` 设置外部数据适配器。
- `xgeXuiTreeViewRefreshAdapter(tree)` 从适配器刷新节点。
- `xgeXuiTreeViewSetNodeExpanded(tree, id, expanded)` 设置展开状态。
- `xgeXuiTreeViewSetNodeEnabled(tree, id, enabled)` 设置节点启用状态。
- `xgeXuiTreeViewSetNodeChecked(tree, id, checked)` 设置 checkbox 状态。
- `xgeXuiTreeViewSetNodeDecorations(tree, id, icon, check)` 设置节点图标和 checkbox 装饰。
- `xgeXuiTreeViewSetSelected(tree, id)` 设置当前选择，并在布局可用后滚动到可见区域。
- `xgeXuiTreeViewSetFont(tree, font)` 设置字体。
- `xgeXuiTreeViewSetMetrics(tree, itemHeight, indent)` 设置行高和层级缩进。
- `xgeXuiTreeViewSetScroll(tree, scrollY)` 设置纵向滚动。
- `xgeXuiTreeViewSetScrollbarMode(tree, mode)` 在完整滚动条和精简滚动条之间切换。
- `xgeXuiTreeViewSetSelect(tree, proc, user)` 设置选择回调。
- `xgeXuiTreeViewSetItemRenderer(tree, proc, user)` 设置自定义行渲染器。
- `xgeXuiTreeViewSetColors(tree, background, row, selected, text, bar, thumb)` 设置基础配色。
- `xgeXuiTreeViewSetDisabledTextColor(tree, color)` 设置禁用节点文字色。

## 交互

- 鼠标点击普通行会选择节点。
- 鼠标点击 expander 会展开或折叠有子节点的节点。
- 鼠标点击 checkbox 会切换当前节点勾选状态。
- `Up` / `Down` 移动选择。
- `PageUp` / `PageDown` 按当前可见行数跳转。
- `Home` / `End` 跳转到首尾可选节点。
- `Right` 展开当前节点，已展开时进入第一个可选子节点。
- `Left` 折叠当前节点，已折叠时回到父节点。
- `Enter` / `Space` 对当前选择触发 select 回调。

## 自定义绘制

`xgeXuiTreeViewSetItemRenderer` 可完全接管行绘制。回调会收到 widget、节点 id、可见行索引、节点指针、行 rect 和状态位：

- `XGE_XUI_TREE_ITEM_SELECTED`
- `XGE_XUI_TREE_ITEM_HOVER`
- `XGE_XUI_TREE_ITEM_DISABLED`
- `XGE_XUI_TREE_ITEM_FOCUS`
- `XGE_XUI_TREE_ITEM_EXPANDED`
- `XGE_XUI_TREE_ITEM_HAS_CHILDREN`
- `XGE_XUI_TREE_ITEM_CHECKED`

回调返回非 0 表示已处理该行，TreeView 不再执行默认行绘制。

自定义行渲染器是整行接管机制。如果控件仍需要显示 expander、checkbox、icon 或其它节点装饰，renderer 必须根据 `XGE_XUI_TREE_ITEM_HAS_CHILDREN`、`XGE_XUI_TREE_ITEM_EXPANDED`、`XGE_XUI_TREE_ITEM_CHECKED` 等状态自行绘制；TreeView 仍会负责点击 expander/checkbox 的事件命中和状态更新。

## XSON

`treeView` 已恢复 XSON 支持，常用字段：

- `nodes`
- `selected`
- `itemHeight`
- `indent`
- `scrollY`
- `scrollbarMode`
- `backgroundColor` / `background`
- `rowColor` / `color`
- `selectedColor`
- `textColor`
- `disabledTextColor`
- `expanderColor`
- `iconColor`
- `checkColor`
- `barColor`
- `thumbColor`

`nodes` 中的每个节点支持字段：

- `id`
- `parent`
- `text`
- `expanded`
- `enabled`
- `icon`
- `check`
- `checked`

## 范例

- `examples/xui_treeview`
- `examples/xui_treeview_xson`
