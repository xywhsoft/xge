# XUI Toast

`examples/xui_toast` 覆盖 `Toast` 的 context 队列、类型、点击回调、关闭按钮、关闭原因、过期移除和屏幕位置。

## 覆盖 API

- `xgeXuiToastShow`
- `xgeXuiToastClose`
- `xgeXuiToastClear`
- `xgeXuiToastSetMetrics`
- `xgeXuiToastSetPlacement`
- `xgeXuiToastSetDirection`
- `xgeXuiToastSetClose`
- `xgeXuiToastGetActiveCount`
- `xgeXuiToastGetPendingCount`

## 构建和运行

```bat
examples\xui_toast\build.bat
build\xui_toast.exe --frames 2
```

## 自动检查点

- `queue=1`：可见队列和等待队列数量正确。
- `click=1`：点击正文触发回调并按 click reason 关闭。
- `expire=1`：短 duration toast 只在进入可见队列后开始计时，并按 timeout reason 关闭。
- `active` / `pending` / `shown` / `closed` / `expired` / `dropped`：输出当前队列和生命周期计数。

## 通过标准

smoke 要求 `queue=1`、`click=1`、`expire=1`；退出码会绑定这三个检查。
