# XUI Table View Lab

`examples/xui_table_view_lab` 覆盖 `TableView` 的列定义、数据适配器、虚拟行访问、选择、排序和列宽调整。

## 覆盖 API

- `xgeXuiTableViewInit`
- `xgeXuiTableViewSetFont`
- `xgeXuiTableViewSetColumns`
- `xgeXuiTableViewSetAdapter`
- `xgeXuiTableViewSetSort`
- `xgeXuiTableViewSetSelect`
- `xgeXuiTableViewSetMetrics`
- `xgeXuiTableViewSetScroll`
- `xgeXuiTableViewGetRowCount`
- `xgeXuiTableViewGetFirstVisible`
- `xgeXuiTableViewGetPaintVisibleCount`
- `xgeXuiTableViewGetSelected`

## 构建和运行

```bat
examples\xui_table_view_lab\build.bat
build\xge_xui_table_view_lab.exe --frames 5
```

## 自动检查点

- `init=1`：列数和行数正确。
- `virtual=1`：滚动后只访问可见行附近的数据。
- `select=1`：点击行可更新 selected 并触发回调。
- `sort=1`：点击表头可触发排序回调。
- `resize=1`：列宽调整路径可运行。

## 通过标准

程序自动退出，并打印 `xui-table-view-lab final-summary`，其中 `init`、`virtual`、`select`、`sort`、`resize` 均为 `1`。

