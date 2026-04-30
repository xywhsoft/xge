# XUI Toast Lab

`examples/xui_toast_lab` 覆盖 `Toast` 的通知队列、类型、关闭按钮、过期移除和屏幕位置。

## 覆盖 API

- `xgeXuiToastInit`
- `xgeXuiToastSetMetrics`
- `xgeXuiToastSetPlacement`
- `xgeXuiToastGetCount`

## 构建和运行

```bat
examples\xui_toast_lab\build.bat
build\xge_xui_toast_lab.exe --frames 2
```

## 自动检查点

- `init=1`：初始 toast 队列和 show 计数正确。
- `close=1`：关闭按钮可移除指定 toast 并更新关闭计数。
- `expire=1`：lab 会确定性推进短 duration toast，验证它可按 duration 过期移除。
- `count` / `show` / `closed` / `expired`：输出当前队列和生命周期计数。

## 通过标准

smoke 要求 `init=1`、`close=1`、`expire=1`；退出码会绑定这三个检查。
