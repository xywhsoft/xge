# Input State Lab

`examples/input_state_lab` 是输入状态综合范例，覆盖键盘、鼠标、剪贴板、触摸和手柄 API。键盘和鼠标保持实时采样；剪贴板、触摸和手柄在程序内模拟，因此可以纳入自动 smoke。

## 覆盖 API

- `xgeKeyDown`
- `xgeKeyPressed`
- `xgeKeyReleased`
- `xgeMouseGet`
- `xgeMouseGetDelta`
- `xgeMouseGetWheel`
- `xgeMouseDown`
- `xgeClipboardSetText`
- `xgeClipboardGetText`
- `xgeTouchGetCount`
- `xgeTouchGet`
- `xgeTouchFind`
- `xgeGamepadConnected`
- `xgeGamepadGetState`
- `xgeGamepadButtonDown`
- `xgeGamepadButtonPressed`
- `xgeGamepadButtonReleased`
- `xgeGamepadAxis`
- `xgeGamepadSetConnected`
- `xgeGamepadSetState`

## 构建和运行

```bat
examples\input_state_lab\build.bat
build\xge_input_state_lab.exe --frames 120
```

可用 `--font <path>` 指定面板字体。没有字体时仍会执行输入 API 和控制台验证，只是窗口文本不会绘制。

## 验证点

- 控制台输出 `input-state-lab init`。
- `clipboard=1` 表示剪贴板写入和读取通过。
- `touch(count=1 get=1 find=1 ...)` 表示模拟 touch 可通过 count/get/find 查询。
- `gamepad(conn=1 state=1 down=1 pressed=1 released=1 axis=1 ...)` 表示模拟手柄连接、状态、按键边沿和轴值通过。
- 移动鼠标、滚轮或按下空格，窗口中的实时采样值会变化。
