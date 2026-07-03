# Platform Runtime Lab

`examples/platform_runtime_lab` 是平台运行时计数的独立范例。它展示 `xgePlatformRuntimeGet`、平台能力、高 DPI 当前值，以及 key/text/mouse/touch/gamepad/resize/quit 计数；自动模式下还会模拟一次 gamepad connect/state/disconnect，用来给 CI 提供稳定的非零 runtime 计数。

## 覆盖 API

- `xgePlatformCapsGet`
- `xgePlatformRuntimeGet`
- `xgeGpuCapsGet`
- `xgeGamepadSetConnected`
- `xgeGamepadSetState`

## 构建和运行

```bat
examples\platform_runtime_lab\build.bat
build\xge_platform_runtime_lab.exe --frames 120
```

## 验证点

- 控制台输出 `platform-runtime-lab init`。
- `caps=1 gpu=1 runtime=1 ci=1` 表示平台能力、GPU 能力、runtime 快照和 CI 标准入口都初始化成功。
- `gamepad_sim=1` 表示自动模式下的 gamepad connect/state/disconnect 三步都成功。
- `counts(...)` 中 `gamepad` 在自动模式下应大于 0；手工移动鼠标、敲键盘、输入文本、缩放窗口后，其余计数应递增。
- 窗口内应能看到 runtime、计数和 caps 三列面板。

## 可作为 CI 入口

- 该示例支持 `--frames` 和 `--seconds`。
- 自动模式不依赖人工输入也能产生稳定 summary。
- 平台相关 CI 可以把它作为 runtime/counter 标准冒烟入口。
