# XUI TreeView

`examples/xui_treeview` 和 `examples/xui_treeview_xson` 覆盖 `TreeView` 的节点层级、展开折叠、选择、禁用节点、checkbox、滚动条模式、自定义行绘制、键盘导航和 XSON 加载。

## 覆盖 API

- `xgeXuiTreeViewInit`
- `xgeXuiTreeViewSetFont`
- `xgeXuiTreeViewSetSelect`
- `xgeXuiTreeViewSetMetrics`
- `xgeXuiTreeViewSetNodeExpanded`
- `xgeXuiTreeViewSetNodeEnabled`
- `xgeXuiTreeViewSetNodeChecked`
- `xgeXuiTreeViewSetNodeDecorations`
- `xgeXuiTreeViewSetScrollbarMode`
- `xgeXuiTreeViewSetItemRenderer`
- `xgeXuiTreeViewGetNodeExpanded`
- `xgeXuiTreeViewGetNodeEnabled`
- `xgeXuiTreeViewGetNodeChecked`
- `xgeXuiTreeViewGetVisibleCount`
- `xgeXuiTreeViewGetVisibleNodeId`
- `xgeXuiTreeViewGetSelected`

## 构建和运行

```bat
examples\xui_treeview\build.bat
build\xui_treeview.exe --frames 5
examples\xui_treeview_xson\build.bat
build\xui_treeview_xson.exe --frames 5
```

## 自动检查点

- `create=1`：TreeView 已创建并接入 widget 事件。
- `layout=1`：布局尺寸有效。
- `state=1`：节点、展开、禁用、勾选和初始选择正确。
- `interaction=1`：鼠标选择、checkbox 切换、折叠和键盘选择正确。
- `scroll=1`：ScrollFrame 滚动、首行和绘制行窗口正确。
- `custom=1`：自定义行渲染器被调用。

## 通过标准

程序自动退出，并打印 `xui_treeview final-summary` 和 `xui_treeview_xson final-summary`，其中核心状态字段均为 `1`。

