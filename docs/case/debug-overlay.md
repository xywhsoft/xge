# Debug Overlay

本案例展示如何把 frame stats、GPU caps 和平台状态绘制到调试层。

[返回范例解析](README.md) | [错误日志教程](../guide/error-and-log.md) | [Graphics API](../api/graphics.md)

## 问题

跨平台开发需要快速看到 draw call、batch、纹理上传、GPU 名称、窗口尺寸和 DPI。Debug overlay 只在开发期启用。

## 读取统计

```c
xge_debug_stats_t stats;

if ( xgeDebugGetStats(&stats) == XGE_OK ) {
	/* format stats */
}
```

## 读取平台状态

```c
xge_platform_runtime_t rt;
xge_gpu_caps_t gpu;

xgePlatformRuntimeGet(&rt);
xgeGpuCapsGet(&gpu);
```

## 绘制

```c
xgeTextDraw(&font, text, 8.0f, 8.0f, xgeColorRGBA(255, 255, 255, 255));
```

Overlay 应最后绘制，并避免影响主场景 blend、camera 和 depth 状态。

## 常见失败原因

发布版仍显示：用编译宏或 debug 开关控制。

统计不准：没有在采样周期开始调用 `xgeFrameStatsReset`。

影响性能：overlay 每帧格式化大量字符串，应限制输出规模。
