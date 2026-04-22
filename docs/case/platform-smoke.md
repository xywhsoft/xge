# 平台后端冒烟

本案例展示如何用同一套步骤验证桌面、Web、Android、小程序和离屏后端的基本可用性。

[返回范例解析](README.md) | [平台教程](../guide/platform-intro.md) | [Platform API](../api/platform.md)

## 问题

XGE 的平台目标很多，不能只靠某个平台上的窗口截图判断整体状态。平台冒烟测试要覆盖：初始化、GL context、framebuffer、输入事件、音频状态、退出流程和能力输出。

## 冒烟目标

```text
init
  -> runtime caps
  -> gpu caps
  -> render clear + shape
  -> input sample
  -> optional audio ready
  -> shutdown
```

## 步骤 1：输出平台能力

```c
static void DumpPlatform(void)
{
	xge_platform_runtime_t runtime;
	xge_gpu_caps_t gpu;
	char caps[4096];

	if ( xgePlatformRuntimeGet(&runtime) == XGE_OK ) {
		xgeLogWrite(XGE_LOG_INFO, "platform", runtime.sName);
	}

	if ( xgeGpuCapsGet(&gpu) == XGE_OK ) {
		xgeLogWrite(XGE_LOG_INFO, "gpu", gpu.sRenderer);
	}

	if ( xgeDebugDumpCaps(caps, sizeof(caps)) > 0 ) {
		xgeLogWrite(XGE_LOG_INFO, "caps", caps);
	}
}
```

## 步骤 2：绘制可识别画面

```c
static int SmokeFrame(void* pUser)
{
	(void)pUser;

	xgeClear(xgeColorRGBA(16, 20, 28, 255));
	xgeShapeRectFill((xge_rect_t){ 24.0f, 24.0f, 128.0f, 80.0f }, xgeColorRGBA(64, 128, 240, 255));
	xgeShapeCircleFill(220.0f, 64.0f, 40.0f, xgeColorRGBA(240, 96, 80, 255));

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	return 0;
}
```

画面需要使用高对比颜色和简单 shape，便于人工验证、截图比对和板卡串流观察。

## 步骤 3：记录结果

平台验证结果写入维护者文档：

```text
dev/docs/平台后端验证结果.md
```

建议每次记录包含平台、系统版本、GPU、后端、脚本、结果和问题摘要。

## 各平台重点

| 平台 | 重点 |
| --- | --- |
| Windows | Sokol window、OpenGL 3.3、输入和音频 |
| Linux 桌面 | X11/Wayland、OpenGL 或 EGL 路径 |
| macOS | OpenGL 可用性和窗口 lifecycle |
| Android | GLES3、触控、音频权限、生命周期 |
| iOS | OpenGL ES 可用性和系统限制 |
| Web | WebGL2、浏览器音频启动策略 |
| 小程序 | JS 桥接、canvas、触控、音频命令 |
| 板卡 Linux | EGL/GBM、DRM/KMS、输入设备 |
| 离屏 | EGL pbuffer/FBO、read pixels |

## 关键 API

| API | 作用 |
| --- | --- |
| `xgePlatformCapsGet` | 获取平台能力 |
| `xgePlatformRuntimeGet` | 获取运行时状态 |
| `xgeGpuCapsGet` | 获取 GPU 能力 |
| `xgeDebugDumpCaps` | 输出调试能力报告 |
| `xgeLogWrite` | 写入日志 |
| `xgeRenderThreadCapsGet` | 查询 render thread 能力 |

## 常见问题

如果某个平台没有开发条件，不要在正式文档里标为失败，应在验证结果中标为“待验证”。

如果窗口可见但输入无效，优先检查平台后端是否把事件转换为 `xge_event_t` 并进入 XGE 输入系统。
