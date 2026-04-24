# XUI Combo Policy Lab

`xui_combo_policy_lab` 把 `ComboBox` 的模型同步和弹层布局策略单独拆出来，重点覆盖 popup/list 选中同步、live drop-down height 更新、`SetItems` 缩减后的 selected reset，以及宽布局 reopen 后 popup 宽度跟随 widget。

## 覆盖 API

- `xgeXuiComboBoxInit`
- `xgeXuiComboBoxUnit`
- `xgeXuiComboBoxSetItems`
- `xgeXuiComboBoxSetFont`
- `xgeXuiComboBoxSetSelect`
- `xgeXuiComboBoxSetSelected`
- `xgeXuiComboBoxGetSelected`
- `xgeXuiComboBoxSetDropDownHeight`
- `xgeXuiComboBoxSetColors`
- `xgeXuiComboBoxIsOpen`
- `xgeXuiComboBoxEvent`
- `xgeXuiComboBoxEventProc`

## 运行方式

```bat
examples\xui_combo_policy_lab\build.bat
build\xge_xui_combo_policy_lab.exe --frames 5
```

## 自动检查

- 初始化后检查 combo widget 的 `procEvent/procPaint`、popup/list widget、默认 `selected/open`。
- 配置 items、font、callback、selected 和 drop-down height。
- 第一次打开时验证 popup 宽度跟随 widget，list selected 会同步到 combo selected。
- 在 popup 打开期间修改 drop-down height，验证高度会立即生效。
- outside click 关闭 popup 后，focus 应回到 combo widget。
- 宽布局 reopen 后，popup 宽度应随新的 widget 宽度更新。
- `SetItems` 把 item count 缩到 2 时，当前 `selected` 应被自动清空；恢复 items 后重新选择 `Bravo`。
- 最后再次打开，验证 popup 仍以当前 selected 和新布局打开。

## 人工观察

- 顶部状态条里的 `init`、`config`、`sync`、`height`、`reset`、`layout`、`select`、`close` 都应为 `1`。
- 结束前 combo 下拉应保持打开，而且宽度比最初更宽。
- 下拉列表当前高亮应落在 `Bravo`。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
