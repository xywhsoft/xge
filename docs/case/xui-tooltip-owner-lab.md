# XUI Tooltip Owner Lab

`examples/xui_tooltip_owner_lab` 专门覆盖 `Tooltip` 的 owner 挂接和状态控制。它把 `TooltipInit/Unit`、`SetText/SetColors/SetOffset/SetEnabled/SetOpen`、`IsOpen`，以及 `xgeXuiTooltipOwnerEventProc` 的 owner capture 链串成了一个可自动退出的自检页。

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

## 构建和运行

```bat
examples\xui_tooltip_owner_lab\build.bat
build\xge_xui_tooltip_owner_lab.exe --frames 5
```

## 自动检查点

- `init=1`：tooltip 初始化后会替换 owner 的 capture proc，同时保留旧 capture 回调链。
- `open=1`：移动到 owner 上方能打开 tooltip，按下鼠标会关闭。
- `offset=1`：修改 offset 后，打开状态下的 popup rect 会立即跟着重排。
- `enabled=1`：禁用 tooltip 时会强制关闭，并阻止后续手动 `SetOpen(1)` 生效。
- `text=1`：空文本不会被强制打开；颜色与文本配置会同步到 tooltip 状态。
- `direct=1`：直接调用 `xgeXuiTooltipOwnerEventProc` 也会走到旧 capture 链。

## 人工观察点

- 主面板里应能看到一个蓝色 `Tooltip target`。
- 初始自动检查结束后，下方应该留着一个可见 tooltip 气泡，位置略偏向 owner 左上角之后再向下展开。
- 你手动把鼠标移出 owner，再移回去时，tooltip 应该跟着消失和出现，不应出现残留气泡。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-tooltip-owner-lab final-summary`。
- `init`、`open`、`offset`、`enabled`、`text`、`direct` 均为 `1`。
