# Core 生命周期

本案例展示 XGE runtime 的初始化、手动帧循环、窗口主循环、计时和 frame stats 读取。它适合作为学习生命周期 API 的第一站，也可以作为自动 smoke 的轻量入口。

[返回范例解析](README.md) | [Core API](../api/core.md)

## 问题

你希望确认 XGE 的两种运行模式都能工作：

- `XGE_RUN_MANUAL`：应用自己驱动 `xgeFrame`，适合工具、宿主循环或无窗口探针。
- `XGE_RUN_GAME_LOOP`：XGE 进入平台后端主循环，适合普通窗口程序。

## 运行方式

```bat
examples\core_lifecycle\build.bat
build\xge_core_lifecycle.exe --frames 180
```

Unix-like 平台：

```sh
./examples/core_lifecycle/build.sh
./build/xge_core_lifecycle --frames 180
```

## 覆盖 API

| API | 用途 |
| --- | --- |
| `xgeInit` | 初始化 runtime |
| `xgeRun` | 注册 frame callback，并在窗口模式进入主循环 |
| `xgeFrame` | 手动模式推进一帧 |
| `xgeRender` | 手动模式触发一次渲染通道 |
| `xgeQuit` | 请求退出 |
| `xgeUnit` | 释放 runtime |
| `xgeSetTitle` | 更新窗口标题 |
| `xgeGetWidth` / `xgeGetHeight` | 读取当前渲染尺寸 |
| `xgeGetDelta` / `xgeGetFPS` | 读取帧时间和 FPS |
| `xgeTimer` / `xgeTimeNow` | 读取相对时间和系统时间 |
| `xgeSleep` | 简短休眠，验证宿主 sleep API |
| `xgeFrameStatsReset` / `xgeFrameStatsGet` | 重置并读取帧统计 |

## 通过标准

- 构建成功并生成 `build\xge_core_lifecycle.exe`。
- 启动后先打印 `manual-summary`，随后打开窗口。
- 窗口中能看到深色背景、进度条和橙色圆形动画。
- 达到 `--frames` 或 `--seconds` 限制后自动退出，并打印 `final-summary` 与总 `summary`。
- 按 `ESC` 能提前退出并打印 `esc-summary`。

## 可跳过条件

- 当前机器没有可用窗口/GL 环境时，可以只记录构建通过，窗口运行标记为跳过。
- CI 环境没有桌面会话时，建议暂不运行该窗口范例。
