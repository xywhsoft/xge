# XUI Data Controls Lab

`examples/xui_data_controls_lab` 是数据型控件总览示例，在同一页面展示 `TreeView`、`TableView` 和 `PropertyGrid`。它用于快速确认数据控件的初始化、数据适配器、可见行和选中状态。

## 覆盖范围

- `TreeView`：层级节点、展开状态、选中节点。
- `TableView`：列定义、虚拟数据适配器、选中行。
- `PropertyGrid`：分类、属性行、编辑器类型、选中属性。

## 构建和运行

```bat
examples\xui_data_controls_lab\build.bat
build\xge_xui_data_controls_lab.exe --frames 5
```

## 自动检查点

- `tree=1`：TreeView 可见节点和 selected 节点正确。
- `table=1`：TableView 列数、行数和 selected 行正确。
- `grid=1`：PropertyGrid 可见项和 selected 属性正确。
- `adapter=1`：TableView 数据适配器被调用。
- `selection=1`：三个数据控件的选中状态都保持在预期值。

## 通过标准

程序自动退出，并打印 `xui-data-controls-lab final-summary`，其中 `tree`、`table`、`grid`、`adapter`、`selection` 均为 `1`。

