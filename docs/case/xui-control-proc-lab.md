# XUI Control Proc Lab

`examples/xui_control_proc_lab` 专门覆盖一组仍然偏底层的 XUI public proc API。它把 `Toggle / CheckBox / Radio / Switch / Slider / ScrollBar / Splitter / Progress` 放进一个可自动退出的面板里，直接调用对应的 `EventProc / PaintProc` 做自检。

## 覆盖 API

- `xgeXuiCheckBoxEventProc`
- `xgeXuiCheckBoxPaintProc`
- `xgeXuiRadioEventProc`
- `xgeXuiRadioPaintProc`
- `xgeXuiRadioGetState`
- `xgeXuiSliderEventProc`
- `xgeXuiSliderPaintProc`
- `xgeXuiScrollBarEventProc`
- `xgeXuiScrollBarPaintProc`
- `xgeXuiSplitterEventProc`
- `xgeXuiSplitterPaintProc`
- `xgeXuiProgressPaintProc`
- `xgeXuiSwitchEventProc`
- `xgeXuiSwitchPaintProc`

## 构建和运行

```bat
examples\xui_control_proc_lab\build.bat
build\xge_xui_control_proc_lab.exe --frames 5
```

## 自动检查点

- `toggle=1`：toggle 的 `procEvent/procPaint` 绑定正确，鼠标点击后 checked 变为 1。
- `checkbox=1`：checkbox 的 `procEvent/procPaint` 绑定正确，鼠标点击后 checked 变为 1。
- `radio=1`：radio 的 `procEvent/procPaint` 绑定正确，组选择切到 B，`xgeXuiRadioGetState` 能看到 hover 状态。
- `switch=1`：switch 的 `procEvent/procPaint` 绑定正确，点击后 checked 变为 1。
- `slider=1`：slider 的 `procEvent/procPaint` 绑定正确，drag 后 value 明显增大。
- `scroll=1`：scrollbar 的 `procEvent/procPaint` 绑定正确，track click + wheel 后 value 增大。
- `split=1`：splitter 的 `procEvent/procPaint` 绑定正确，drag 后 value 明显变化。
- `progress=1`：progress 的 `procPaint` 绑定正确，range/value 保持稳定。
- `paint=1`：本例已直接调用过全部公开 paint proc 一次。

## 人工观察点

- 左侧四个二值控件应当都有 hover/active 反馈，radio B 最终应选中。
- 中间 slider 应停在明显靠右的位置，右侧 scrollbar 应停在中下段。
- splitter 拖动后，滑块位置应比初始值更靠下。
- progress 条应保持固定进度，不应闪烁或跑偏。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-control-proc-lab final-summary`。
- `toggle`、`checkbox`、`radio`、`switch`、`slider`、`scroll`、`split`、`progress`、`paint` 均为 `1`。
