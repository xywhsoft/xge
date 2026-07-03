# XUI Property Grid Lab

`examples/xui_property_grid_lab` 覆盖 `PropertyGrid` 的分类、属性行、标记状态、选择、折叠和编辑器类型。

## 覆盖 API

- `xgeXuiPropertyGridInit`
- `xgeXuiPropertyGridSetFont`
- `xgeXuiPropertyGridSetMetrics`
- `xgeXuiPropertyGridSetSelect`
- `xgeXuiPropertyGridSetPropertyFlags`
- `xgeXuiPropertyGridGetVisibleCount`
- `xgeXuiPropertyGridGetSelected`

## 构建和运行

```bat
examples\xui_property_grid_lab\build.bat
build\xge_xui_property_grid_lab.exe --frames 5
```

## 自动检查点

- `init=1`：分类和属性行初始化完成，可见行数正确。
- `flags=1`：readonly、changed、error 标记可设置。
- `select=1`：点击属性行可更新 selected 并触发回调。
- `collapse=1`：分类展开/折叠会更新可见行数。
- `editors=1`：文本、数字、布尔、枚举、颜色编辑器类型可配置。
- `row=1`：行定位和行索引计算正确。

## 通过标准

程序自动退出，并打印 `xui-property-grid-lab final-summary`，其中 `init`、`flags`、`select`、`collapse`、`editors`、`row` 均为 `1`。

