# XUI Tree View Lab

`examples/xui_tree_view_lab` 覆盖 `TreeView` 的节点层级、展开折叠、选择、键盘导航和回调。

## 覆盖 API

- `xgeXuiTreeViewInit`
- `xgeXuiTreeViewSetFont`
- `xgeXuiTreeViewSetSelect`
- `xgeXuiTreeViewSetMetrics`
- `xgeXuiTreeViewSetNodeExpanded`
- `xgeXuiTreeViewGetNodeExpanded`
- `xgeXuiTreeViewGetVisibleCount`
- `xgeXuiTreeViewGetVisibleNodeId`
- `xgeXuiTreeViewGetSelected`

## 构建和运行

```bat
examples\xui_tree_view_lab\build.bat
build\xge_xui_tree_view_lab.exe --frames 5
```

## 自动检查点

- `init=1`：树节点和可见节点序列正确。
- `expand=1`：指定节点可展开。
- `select=1`：鼠标选择能更新 selected 并触发回调。
- `keyboard=1`：键盘导航可移动选择。
- `collapse=1`：折叠节点时选择状态保持在有效节点上。

## 通过标准

程序自动退出，并打印 `xui-tree-view-lab final-summary`，其中 `init`、`expand`、`select`、`keyboard`、`collapse` 均为 `1`。

