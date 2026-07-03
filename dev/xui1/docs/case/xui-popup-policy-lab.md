# XUI Popup Policy Lab

`xui_popup_policy_lab` 是 Popup 旧关闭策略回归范例。新 Popup 设计口径以 [docs/xui/popup.md](../xui/popup.md) 为准。

本范例把旧 `Popup` 的关闭策略单独拆出来，重点不是 owner 接线，而是 `outside` / `escape` 两组 policy 组合的行为差异，以及它们对 `visible`、`focus`、`close callback` 的影响。

## 覆盖 API

- `xgeXuiPopupInit`
- `xgeXuiPopupUnit`
- `xgeXuiPopupSetOwner`
- `xgeXuiPopupSetClose`
- `xgeXuiPopupSetOpen`
- `xgeXuiPopupIsOpen`
- `xgeXuiPopupSetAutoClose`
- `xgeXuiPopupSetBackground`
- `xgeXuiPopupEvent`
- `xgeXuiPopupEventProc`
- `xgeXuiPopupPaintProc`

## 运行方式

```bat
examples\xui_popup_policy_lab\build.bat
build\xge_xui_popup_policy_lab.exe --frames 5
```

## 自动检查

- 初始化后检查 `procEvent/procPaint`、默认 `closeOnOutside/closeOnEscape`、focusable、visible 和 clip 标记。
- 关闭全部 auto-close 后，outside click 和 `ESC` 都不应关闭 popup。
- 打开全部 auto-close 时，点击 owner 不应误关 popup。
- 设成 `outside-only` 后，`ESC` 不应关闭，但 outside click 应关闭并触发 close callback。
- 设成 `escape-only` 后，outside click 不应关闭，但 `ESC` 应通过 `PopupEventProc` 关闭并触发 close callback。
- 最后重新打开 popup，验证 `SetBackground`、`visible` 和 focus 状态。
- 退出前打印 `xui-popup-policy-lab final-summary`。

## 人工观察

- 顶部状态条里的 `init`、`none`、`owner`、`outside`、`escape`、`bg`、`visible` 都应为 `1`。
- 左侧 owner 区和右侧 popup 都应稳定可见。
- 结束前 popup 应保持打开，状态条里的 `policy` 应显示 `1/1`。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
