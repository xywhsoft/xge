# XUI Button State Lab

`examples/xui_button_state_lab` 专门覆盖 `Button` 和 `IconButton` 的状态流。它把 hover、active、focus、disabled、capture、keyboard click 这些路径拆成一个可自动退出的自检页，同时把最终状态直接显示在界面里，方便人工观察。

## 覆盖 API

- `xgeXuiButtonInit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`
- `xgeXuiButtonEvent`
- `xgeXuiButtonPaintProc`
- `xgeXuiIconButtonInit`
- `xgeXuiIconButtonSetClick`
- `xgeXuiIconButtonSetTexture`
- `xgeXuiIconButtonSetSource`
- `xgeXuiIconButtonSetIconColor`
- `xgeXuiIconButtonSetMode`
- `xgeXuiIconButtonSetColors`
- `xgeXuiIconButtonGetState`
- `xgeXuiIconButtonEvent`
- `xgeXuiIconButtonPaintProc`
- `xgeXuiDispatchEvent`

## 构建和运行

```bat
examples\xui_button_state_lab\build.bat
build\xge_xui_button_state_lab.exe --frames 5
```

## 自动检查点

- `init=1`：button/icon button 的 proc 绑定、focusable、setter 数据都落到控件实例里。
- `button=1`：直接 `xgeXuiButtonEvent` 路径能跑通 hover、mouse down/up、focus、capture 和 callback。
- `dispatch=1`：通过 `xgeXuiDispatchEvent` 路径能驱动 icon button 的 hover、active、focus、keyboard click。
- `icon=1`：icon button 在清掉 hover 后会保留 focus 位，便于观察 focus-only 状态。
- `disabled=1`：禁用后再次发送 mouse down，不会消费事件，状态会带上 disabled。
- `paint=1`：本例已直接调用过 `xgeXuiButtonPaintProc` 和 `xgeXuiIconButtonPaintProc`。

## 人工观察点

- 左侧文本按钮最终应停在 hover 状态，下面状态行会显示 `button: hover (...)`。
- 右侧图标按钮最终应停在 focus 状态，下面状态行会显示 `icon: normal|focus (...)`。
- 两个控件的底色应该明显不同，方便分辨 hover 和 focus 两种视觉反馈。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-button-state-lab final-summary`。
- `init`、`button`、`icon`、`dispatch`、`disabled`、`paint` 均为 `1`。
