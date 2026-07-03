# XUI TableView

`examples/xui_tableview` 和 `examples/xui_tableview_xson` 覆盖新 `TableView` 口径。旧 `xui_table_view_lab` 已被替换，不再作为维护目标。

## 覆盖 API

- `xgeXuiTableViewInit`
- `xgeXuiTableViewSetColumns`
- `xgeXuiTableViewSetRows`
- `xgeXuiTableViewSetAdapter`
- `xgeXuiTableViewSetSelectionMode`
- `xgeXuiTableViewSetSelectedRow`
- `xgeXuiTableViewSetSelectedCell`
- `xgeXuiTableViewSetDefaultMetrics`
- `xgeXuiTableViewSetScrollbarMode`
- `xgeXuiTableViewSetHeaderRenderer`
- `xgeXuiTableViewSetCellRenderer`
- `xgeXuiTableViewSetColumnFormatter`
- `xgeXuiTableViewSetHover`
- `xgeXuiTableViewGetActiveCell`
- `xgeXuiTableViewGetCellRect`
- `xgeXuiTableViewGetCellContentRect`
- `xgeXuiTableViewGetBodyWidget`
- `xgeXuiTableViewGetViewportWidget`
- `xgeXuiTableViewEnsureCellVisible`

## 构建和运行

```bat
examples\xui_tableview\build.bat
build\xui_tableview.exe --frames 3

examples\xui_tableview_xson\build.bat
build\xui_tableview_xson.exe --frames 3
```

## 通过标准

两个程序自动退出，并分别打印 `xui_tableview final-summary` 和 `xui_tableview_xson final-summary`，其中 `create`、`layout`、`state` 均为 `1`。
