# XUI XSON Controls Lab

`examples/xui_xson_controls_lab` 是 XSON 控件声明的聚合 smoke。它不打开窗口，直接加载一份包含多类控件的 XSON 页面，并检查控件实例计数、关键字段映射和数据适配器内容。

## 覆盖范围

- 基础控件：`label`、`button`、`input`、`numericInput`。
- 选择控件：`checkbox`、`radio`、`toggle`。
- 数值控件：`slider`、`progress`、`tabs`。
- 工具控件：`toolbar`、`statusBar`。
- 数据控件：`treeView`、`tableView`、`propertyGrid`。
- 体验控件：`breadcrumb`、`accordion`、`toast`。

Overlay 控件的 owner、关闭顺序和焦点恢复由 `xui_overlay_policy_lab` 覆盖。

## 构建和运行

```bat
examples\xui_xson_controls_lab\build.bat
build\xge_xui_xson_controls_lab.exe
```

## 自动检查点

- `load=1`：XSON 页面加载成功。
- `basic=1`：基础控件数量和值正确。
- `choice=1`：选择控件 checked 状态正确。
- `range=1`：Slider、Progress、Tabs 字段正确。
- `bars=1`：Toolbar 和 StatusBar 项数量正确。
- `data=1`：TreeView、TableView、PropertyGrid 数据和 selected 状态正确。
- `experience=1`：Breadcrumb、Accordion、Toast 项数量正确。

## 通过标准

程序输出 `xui-xson-controls-lab final-summary`，且 `load`、`basic`、`choice`、`range`、`bars`、`data`、`experience` 均为 `1`。

