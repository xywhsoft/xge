# XUI Breadcrumb Lab

`examples/xui_breadcrumb_lab` 覆盖 `Breadcrumb` 的路径段、分隔符、窄宽度折叠、段点击和选择回调。

## 覆盖 API

- `xgeXuiBreadcrumbInit`
- `xgeXuiBreadcrumbSetFont`
- `xgeXuiBreadcrumbSetSelect`
- `xgeXuiBreadcrumbSetMetrics`
- `xgeXuiBreadcrumbAddSegment`
- `xgeXuiBreadcrumbGetSegmentCount`
- `xgeXuiBreadcrumbGetCollapsedCount`
- `xgeXuiBreadcrumbGetFirstVisible`
- `xgeXuiBreadcrumbGetSelected`

## 构建和运行

```bat
examples\xui_breadcrumb_lab\build.bat
build\xge_xui_breadcrumb_lab.exe --frames 5
```

## 自动检查点

- `init=1`：路径段数量正确。
- `overflow=1`：空间不足时可折叠前置路径段。
- `select=1`：点击可见段可更新 selected 并触发回调。
- `collapsed`：输出当前折叠段数量，便于人工核对窄宽度显示。

## 通过标准

程序自动退出，并打印 `xui-breadcrumb-lab final-summary`，其中 `init`、`overflow`、`select` 均为 `1`。

