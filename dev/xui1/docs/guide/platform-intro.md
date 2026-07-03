# 平台后端与冒烟测试入门

> 状态：中文初稿已生成，待审阅。

XGE 的跨平台能力由平台后端和图形后端共同决定。平台后端负责窗口、事件、输入和 present；图形后端负责 OpenGL/GLES/WebGL 能力。

## 平台和图形是两层

| 层 | 例子 | 负责什么 |
| --- | --- | --- |
| Platform backend | Sokol、MiniProgram、EGL、自定义宿主 | 窗口、事件、输入、present。 |
| Graphics backend | OpenGL 3.3、GLES3、WebGL2 | shader、buffer、texture、draw。 |

普通桌面和移动端默认走 Sokol。小程序、板卡 Linux、离屏渲染会用专门后端或桥接。

## 查询能力

```c
xge_platform_caps_t caps;
if ( xgePlatformCapsGet(&caps) == XGE_OK ) {
	/* caps.bWindow, caps.bTouch, caps.bOffscreen ... */
}
```

查询 GPU：

```c
xge_gpu_caps_t gpu;
if ( xgeGpuCapsGet(&gpu) == XGE_OK ) {
	/* gpu.sRenderer, gpu.iMaxTextureSize ... */
}
```

运行时状态：

```c
xge_platform_runtime_t runtime;
xgePlatformRuntimeGet(&runtime);
```

高 DPI、framebuffer 尺寸和事件计数都从 runtime 里看。

## 平台验证原则

不要把“脚手架存在”当成“平台已支持”。XGE 文档里区分三种状态：

| 状态 | 说明 |
| --- | --- |
| 已实现 | 代码路径存在。 |
| 可开发验证 | 当前机器或常见环境可跑冒烟。 |
| 已实机验证 | 目标平台真实运行通过，并记录结果。 |

实测结果记录在 `dev/docs/平台后端验证结果.md`。正式文档只写策略和入口，不伪造实测结论。

## 本地冒烟

Windows 下：

```bat
check_platform_smoke.bat
```

脚本会构建并运行 platform smoke 示例，检查 caps 和 runtime 输出。

跨平台脚手架检查：

```bat
check_platform_all.bat
```

Linux/macOS 目标上使用对应 `.sh` 脚本。

## 自定义后端

自定义平台后端必须在 `xgeInit` 前设置。

```c
xge_platform_backend_t backend = xgePlatformBackendDefault();
/* fill backend callbacks */
xgePlatformBackendSet(&backend);
```

初始化后切换后端是不合法的。后端负责的最小职责是 init、unit、poll 和 present。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 初始化后切换后端失败 | 后端必须早于 `xgeInit` 设置 | 在启动配置阶段设置。 |
| 高 DPI 下坐标错位 | 逻辑尺寸和 framebuffer 尺寸不同 | 用 `xgePlatformRuntimeGet` 读取 framebuffer 和 DPR。 |
| 文档与实机不一致 | 未记录目标平台实测 | 跑目标脚本，并更新 `dev/docs/平台后端验证结果.md`。 |

## 下一步

- 查完整能力字段看 [Platform API](../api/platform.md)。
- 离屏和板卡路径看 [Offscreen / EGL API](../api/offscreen.md)。

[返回教程入口](README.md)
