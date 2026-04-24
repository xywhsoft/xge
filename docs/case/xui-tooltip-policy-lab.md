# XUI Tooltip Policy Lab

`xui_tooltip_policy_lab` 把 `Tooltip` 的开关规则和 relayout 行为单独拆出来，重点覆盖 `SetOpen` 的 `enabled/text` 门控、打开状态下改 text/offset 的即时重排，以及 owner capture 事件的 open/close 规则。

## 覆盖 API

- `xgeXuiTooltipInit`
- `xgeXuiTooltipUnit`
- `xgeXuiTooltipSetText`
- `xgeXuiTooltipSetColors`
- `xgeXuiTooltipSetOffset`
- `xgeXuiTooltipSetEnabled`
- `xgeXuiTooltipSetOpen`
- `xgeXuiTooltipIsOpen`
- `xgeXuiTooltipOwnerEventProc`

## 运行方式

```bat
examples\xui_tooltip_policy_lab\build.bat
build\xge_xui_tooltip_policy_lab.exe --frames 5
```

## 自动检查

- 初始化后检查默认 `offset=8,6`、enabled 状态，以及 owner capture 是否被 tooltip 接管并保留旧 capture 处理器。
- `sText` 为空时直接 `SetOpen(1)` 不应真的打开。
- 打开状态下把 text 从短串换成长串，验证 popup 会即时变宽；再修改 offset，验证位置即时更新。
- `pointer enter / touch move` 应打开 tooltip；`mouse down / pointer leave` 应关闭 tooltip。
- `SetEnabled(0)` 时 tooltip 应立即关闭，且后续 `SetOpen(1)` 不应生效。
- owner 事件转发过程中，旧 capture handler 仍应被继续调用。
- 退出前重新保持 tooltip 打开，并打印 `xui-tooltip-policy-lab final-summary`。

## 人工观察

- 顶部状态条里的 `init`、`gate`、`relayout`、`owner`、`disable`、`legacy` 都应为 `1`。
- 结束前 tooltip 应保持打开，位置应明显偏在 owner 右下方。
- tooltip 宽度应比最初短文本状态更宽。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
