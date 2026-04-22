# 错误、日志与调试信息

本教程说明 XGE 的错误码、日志、GPU 能力输出和 frame stats 使用方式。

[返回教程索引](README.md) | [Core API](../api/core.md) | [Graphics API](../api/graphics.md)

## 错误返回

XGE 的公开 C API 优先使用 `int` 返回枚举码：

```c
if ( xgeInit(&desc) != XGE_OK ) {
	return 1;
}
```

返回 `void` 的函数通常是状态设置、绘制提交或清理函数，不表达复杂失败原因。

## 日志级别

```c
xgeLogSetLevel(XGE_LOG_INFO);
xgeLogWrite(XGE_LOG_INFO, "app", "startup");
xgeLogFlush();
```

日志后端基于 xrt logger。XGE 模块内部应使用统一 tag，便于按平台和模块定位问题。

## GPU 能力输出

```c
char buffer[4096];

if ( xgeDebugDumpCaps(buffer, sizeof(buffer)) > 0 ) {
	xgeLogWrite(XGE_LOG_INFO, "caps", buffer);
}
```

Debug caps 用于开发期和平台冒烟。发布版不承诺保留完整诊断输出。

## Frame Stats

```c
xge_debug_stats_t stats;

if ( xgeDebugGetStats(&stats) == XGE_OK ) {
	/* draw call, batch, texture switch, upload count */
}
```

统计信息适合 debug overlay、性能调优和人工测试记录。

## 重置统计

```c
xgeFrameStatsReset();
```

在切换场景、开始性能采样或进入测试段前重置统计，结果更容易解释。

## 建议实践

- 初始化失败：输出平台、GPU、后端、脚本名。
- 资源加载失败：输出规范化后的路径和 provider。
- shader 编译失败：输出当前图形后端和 shader header。
- 平台问题：把结果写入 `dev/docs/平台后端验证结果.md`。

## 常见错误

不要把 `XGE_OK` 以外的返回值全部静默吞掉。至少在开发期写日志。

不要在每帧输出大量日志。高频日志会影响帧率，并污染测试结果。

不要把 debug caps 当作发布版用户可见功能。它是诊断工具。

## 下一步

- 平台验证读 [平台后端与冒烟测试入门](platform-intro.md)。
- APP 模式调试读 [运行模式入门](run-mode-intro.md)。
