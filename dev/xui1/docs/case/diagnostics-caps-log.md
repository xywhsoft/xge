# 诊断、Caps 与日志

本案例展示 XGE 的诊断查询、日志级别、平台后端、图形后端、GPU caps、runtime counters 和 shader/library 映射信息。

[返回范例解析](README.md) | [Core API](../api/core.md)

## 问题

你希望在一个可运行窗口中确认当前平台能力，并把关键诊断信息打印到终端，方便人工测试、CI smoke 和平台移植排查。

## 运行方式

```bat
examples\diagnostics_caps_log\build.bat
build\xge_diagnostics_caps_log.exe --frames 120
```

Unix-like 平台：

```sh
./examples/diagnostics_caps_log/build.sh
./build/xge_diagnostics_caps_log --frames 120
```

## 覆盖 API

| API | 用途 |
| --- | --- |
| `xgeDebugGetStats` | 读取纹理、字体、音频和帧统计 |
| `xgeDebugDumpCaps` | 一次性输出 caps 文本 |
| `xgeLogSetLevel` / `xgeLogGetLevel` | 调整和恢复日志级别 |
| `xgeLogWrite` / `xgeLogFlush` | 输出并 flush 诊断日志 |
| `xgePlatformBackendDefault` / `xgePlatformBackendSet` / `xgePlatformBackendGet` | 查询和设置平台后端 |
| `xgeGraphicsBackendDefault` / `xgeGraphicsBackendSet` / `xgeGraphicsBackendGet` | 查询和设置图形后端 |
| `xgeGpuCapsGet` | 读取 GPU 能力 |
| `xgePlatformCapsGet` | 读取平台能力 |
| `xgePlatformRuntimeGet` | 读取窗口、DPI 和输入事件计数 |
| `xgeGraphicsShaderHeaderGet` | 查询 shader header |
| `xgeGraphicsLibraryNameGet` | 查询图形库名称 |
| `xgeGraphicsMappingGet` | 查询后端映射 |

## 通过标准

- 构建成功并生成 `build\xge_diagnostics_caps_log.exe`。
- 启动后终端打印 `diagnostics pre-init`、caps dump、`diagnostics backend` 和 runtime summary。
- 窗口中能看到深色背景、蓝色矩形和黄色圆形。
- 达到 `--frames` 或 `--seconds` 限制后自动退出，并打印 `diagnostics final-summary` 和总 summary。
- 按 `ESC` 能提前退出并打印 `diagnostics esc-summary`。

## 可跳过条件

- 当前环境没有桌面窗口或 GL context 时，可以记录构建通过，运行验证标记为跳过。
- 没有真实 GPU caps 的无窗口/虚拟环境中，`vendor/renderer/version` 为空不视为失败。
