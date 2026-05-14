# XUI Combo Detail Lab

`xui_combo_detail_lab` 是 ComboBox 旧回归范例，保留用于对照历史行为。新控件口径以 `docs/xui/combobox.md`、`examples/xui_combobox` 和 `examples/xui_combobox_xson` 为准。

旧范例重点展示 selected/open/state 的状态流，以及 disabled row、popup height 和 keyboard open 这些更细的行为。

## 覆盖 API

- `xgeXuiComboBoxInit`
- `xgeXuiComboBoxUnit`
- `xgeXuiComboBoxSetItems`
- `xgeXuiComboBoxSetFont`
- `xgeXuiComboBoxSetSelect`
- `xgeXuiComboBoxSetSelected`
- `xgeXuiComboBoxGetSelected`
- `xgeXuiComboBoxSetPopupHeight`
- `xgeXuiComboBoxSetColors`
- `xgeXuiComboBoxIsOpen`
- `xgeXuiComboBoxGetState`
- `xgeXuiComboBoxEvent`
- `xgeXuiComboBoxEventProc`
- `xgeXuiComboBoxPaintProc`
- `xgeXuiListViewSetEnabledItems`

## 运行方式

```bat
examples\xui_combo_detail_lab\build.bat
build\xge_xui_combo_detail_lab.exe --frames 5
```

## 自动检查

- 初始化后检查 `procEvent/procPaint`、popup/list widget 与默认 selected/open 状态。
- 配置 items、font、popup height、colors 和 callback，并把第二项标成 disabled。
- 鼠标点击 combo 后应展开，下拉高度应固定到 `72`。
- 点击 disabled row 不应改变 selected，也不应触发 callback。
- 点击 enabled row 后应关闭下拉，并把 selected 改到 `Gamma`。
- 给 combo widget 焦点后按 `Down` 应再次展开。
- 禁用 combo widget 后，`xgeXuiComboBoxGetState()` 应带 `DISABLED` 标记。

程序退出前会打印 `xui-combo-detail-lab final-summary`。

## 人工观察

- 顶部状态条里的 `init`、`config`、`open`、`disabled`、`select`、`key`、`state` 都应为 `1`。
- 组合框最终应显示 `Gamma`。
- 下拉列表第二项 `Beta (Disabled)` 应明显是 disabled 态。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
